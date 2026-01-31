#include "dbus_manager_adaptor.hpp"
#include <domain/entities/app_image_record.hpp>
#include <domain/entities/config.hpp>
#include <domain/entities/install_type.hpp>
#include <domain/entities/launch_settings.hpp>
#include <domain/repositories/config_repository.hpp>
#include <domain/repositories/registry_repository.hpp>
#include <domain/repositories/launch_settings_repository.hpp>
#include <QDBusArgument>
#include <QCoreApplication>
#include <QStringLiteral>
#include <cassert>
#include <cstdlib>
#include <optional>
#include <unordered_map>
#include <vector>

namespace {

class MockRegistryRepository : public appimage_manager::domain::RegistryRepository {
public:
  std::vector<appimage_manager::domain::AppImageRecord> records;

  std::vector<appimage_manager::domain::AppImageRecord> all() const override { return records; }
  std::optional<appimage_manager::domain::AppImageRecord> by_path(const std::string&) const override { return std::nullopt; }
  std::optional<appimage_manager::domain::AppImageRecord> by_id(const std::string&) const override { return std::nullopt; }
  void save(const appimage_manager::domain::AppImageRecord&) override {}
  void remove_by_path(const std::string&) override {}
  void remove(const std::string&) override {}
};

class MockConfigRepository : public appimage_manager::domain::ConfigRepository {
public:
  appimage_manager::domain::Config load() const override { return {}; }
  void save(const appimage_manager::domain::Config&) override {}
};

class MockLaunchSettingsRepository : public appimage_manager::domain::LaunchSettingsRepository {
public:
  std::optional<appimage_manager::domain::LaunchSettings> load(const std::string&) const override { return std::nullopt; }
  void save(const std::string&, const appimage_manager::domain::LaunchSettings&) override {}
  std::unordered_map<std::string, appimage_manager::domain::LaunchSettings> load_all() const override { return {}; }
  void remove(const std::string&) override {}
};

int test_getallrecords_returns_maps_with_required_keys() {
  MockRegistryRepository registry;
  appimage_manager::domain::AppImageRecord r1;
  r1.id = "id-one";
  r1.name = "FirstApp";
  r1.path = "/opt/First.AppImage";
  r1.install_type = appimage_manager::domain::InstallType::GitHub;
  appimage_manager::domain::AppImageRecord r2;
  r2.id = "id-two";
  r2.name = "SecondApp";
  r2.path = "/home/user/Second.AppImage";
  r2.install_type = appimage_manager::domain::InstallType::Downloaded;
  registry.records = { r1, r2 };

  MockConfigRepository config_repo;
  MockLaunchSettingsRepository launch_repo;
  QObject parent;
  appimage_manager::daemon::DBusManagerAdaptor adaptor(
    registry, config_repo, launch_repo, "/tmp/apps", nullptr, &parent);

  QVariantList list = adaptor.GetAllRecords();
  assert(list.size() == 2u);

  QVariantMap m0 = qdbus_cast<QVariantMap>(list.at(0));
  assert(m0.value(QStringLiteral("id")).toString() == QStringLiteral("id-one"));
  assert(m0.value(QStringLiteral("name")).toString() == QStringLiteral("FirstApp"));
  assert(m0.value(QStringLiteral("path")).toString() == QStringLiteral("/opt/First.AppImage"));
  assert(m0.value(QStringLiteral("install_type")).toString() == QStringLiteral("GitHub"));

  QVariantMap m1 = qdbus_cast<QVariantMap>(list.at(1));
  assert(m1.value(QStringLiteral("id")).toString() == QStringLiteral("id-two"));
  assert(m1.value(QStringLiteral("name")).toString() == QStringLiteral("SecondApp"));
  assert(m1.value(QStringLiteral("path")).toString() == QStringLiteral("/home/user/Second.AppImage"));
  assert(m1.value(QStringLiteral("install_type")).toString() == QStringLiteral("Downloaded"));

  return 0;
}

int test_getallrecords_empty_registry_returns_empty_list() {
  MockRegistryRepository registry;
  MockConfigRepository config_repo;
  MockLaunchSettingsRepository launch_repo;
  QObject parent;
  appimage_manager::daemon::DBusManagerAdaptor adaptor(
    registry, config_repo, launch_repo, "/tmp", nullptr, &parent);

  QVariantList list = adaptor.GetAllRecords();
  assert(list.isEmpty());

  return 0;
}

}

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  if (argc >= 2 && std::atoi(argv[1]) == 1)
    return test_getallrecords_empty_registry_returns_empty_list() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
  return test_getallrecords_returns_maps_with_required_keys() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
