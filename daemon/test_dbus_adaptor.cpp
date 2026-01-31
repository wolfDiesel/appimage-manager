#include "dbus_manager_adaptor.hpp"
#include "desktop_notification.hpp"
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
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

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

class StatefulMockRegistryRepository : public appimage_manager::domain::RegistryRepository {
public:
  std::vector<appimage_manager::domain::AppImageRecord> records;

  std::vector<appimage_manager::domain::AppImageRecord> all() const override { return records; }
  std::optional<appimage_manager::domain::AppImageRecord> by_path(const std::string& path) const override {
    for (const auto& r : records) if (r.path == path) return r;
    return std::nullopt;
  }
  std::optional<appimage_manager::domain::AppImageRecord> by_id(const std::string& id) const override {
    for (const auto& r : records) if (r.id == id) return r;
    return std::nullopt;
  }
  void save(const appimage_manager::domain::AppImageRecord& r) override { records.push_back(r); }
  void remove_by_path(const std::string&) override {}
  void remove(const std::string& id) override {
    records.erase(std::remove_if(records.begin(), records.end(),
      [&id](const appimage_manager::domain::AppImageRecord& r) { return r.id == id; }), records.end());
  }
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

int test_remove_appimage_removes_record_and_returns_true() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-remove";
  fs::create_directories(tmp);
  fs::path app_path = tmp / "fake.AppImage";
  std::ofstream(app_path).put('x');

  StatefulMockRegistryRepository registry;
  appimage_manager::domain::AppImageRecord r;
  r.id = "id-remove-me";
  r.path = app_path.string();
  r.name = "Fake";
  r.install_type = appimage_manager::domain::InstallType::Downloaded;
  registry.records.push_back(r);

  MockConfigRepository config_repo;
  MockLaunchSettingsRepository launch_repo;
  std::string apps_dir = (tmp / "apps").string();
  fs::create_directories(apps_dir);

  QObject parent;
  appimage_manager::daemon::DBusManagerAdaptor adaptor(
    registry, config_repo, launch_repo, apps_dir, nullptr, &parent);

  bool ok = adaptor.RemoveAppImage(QStringLiteral("id-remove-me"));
  assert(ok);
  assert(registry.records.empty());
  assert(!adaptor.GetAllRecords().size());

  fs::remove_all(tmp);
  return 0;
}

int test_notify_appimage_processed_does_not_crash() {
  appimage_manager::daemon::notify_appimage_processed("Test.AppImage", "/tmp");
  return 0;
}

int test_remove_appimage_unknown_id_returns_false() {
  MockRegistryRepository registry;
  MockConfigRepository config_repo;
  MockLaunchSettingsRepository launch_repo;
  QObject parent;
  appimage_manager::daemon::DBusManagerAdaptor adaptor(
    registry, config_repo, launch_repo, "/tmp", nullptr, &parent);

  bool ok = adaptor.RemoveAppImage(QStringLiteral("nonexistent"));
  assert(!ok);
  return 0;
}

}

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  if (argc >= 2) {
    int n = std::atoi(argv[1]);
    if (n == 0) return test_getallrecords_returns_maps_with_required_keys() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    if (n == 1) return test_getallrecords_empty_registry_returns_empty_list() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    if (n == 2) return test_remove_appimage_removes_record_and_returns_true() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    if (n == 3) return test_remove_appimage_unknown_id_returns_false() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    if (n == 4) return test_notify_appimage_processed_does_not_crash() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  if (test_getallrecords_returns_maps_with_required_keys() != 0) return EXIT_FAILURE;
  if (test_getallrecords_empty_registry_returns_empty_list() != 0) return EXIT_FAILURE;
  if (test_remove_appimage_removes_record_and_returns_true() != 0) return EXIT_FAILURE;
  if (test_remove_appimage_unknown_id_returns_false() != 0) return EXIT_FAILURE;
  if (test_notify_appimage_processed_does_not_crash() != 0) return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
