#include <domain/entities/config.hpp>
#include <domain/entities/launch_settings.hpp>
#include <domain/repositories/config_repository.hpp>
#include <domain/repositories/registry_repository.hpp>
#include <domain/repositories/launch_settings_repository.hpp>
#include <application/scan_directories.hpp>
#include <application/generate_desktop.hpp>
#include <infrastructure/json/json_config_repository.hpp>
#include <infrastructure/json/json_registry_repository.hpp>
#include <infrastructure/json/json_launch_settings_repository.hpp>
#include <directory_watcher.hpp>
#include <dbus_manager_adaptor.hpp>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <iostream>
#include <cstdlib>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

std::string default_config_dir() {
  const char* home = std::getenv("HOME");
  if (!home || !*home)
    return "";
  std::string path = home;
  path += "/.config/appimage-manager";
  return path;
}

std::string default_applications_dir() {
  const char* home = std::getenv("HOME");
  if (!home || !*home)
    return "";
  std::string path = home;
  path += "/.local/share/applications";
  return path;
}

}

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  app.setApplicationName(QStringLiteral("appimage-manager-daemon"));

  std::string config_dir = default_config_dir();
  if (config_dir.empty()) {
    std::cerr << "appimage-manager-daemon: HOME not set\n";
    return EXIT_FAILURE;
  }
  std::string applications_dir = default_applications_dir();
  if (applications_dir.empty()) {
    std::cerr << "appimage-manager-daemon: HOME not set\n";
    return EXIT_FAILURE;
  }

  appimage_manager::infrastructure::JsonConfigRepository config_repository(config_dir);
  appimage_manager::domain::Config config = config_repository.load();
  fs::path config_file = fs::path(config_dir) / "config.json";
  if (!fs::is_regular_file(config_file)) {
    fs::create_directories(config_dir);
    config_repository.save(config);
  }
  std::cerr << "appimage-manager-daemon: config from " << config_dir << ", watch_directories: "
            << config.watch_directories.size() << "\n";
  for (const auto& dir : config.watch_directories) {
    fs::path p(dir);
    bool exists = fs::exists(p);
    bool is_dir = fs::is_directory(p);
    std::cerr << "  " << dir << " exists=" << exists << " is_directory=" << is_dir;
    if (!is_dir)
      std::cerr << " (skipped)";
    std::cerr << "\n";
  }
  appimage_manager::infrastructure::JsonRegistryRepository registry(config_dir);
  appimage_manager::infrastructure::JsonLaunchSettingsRepository launch_settings_repository(config_dir);
  appimage_manager::application::ScanDirectories scan(registry);
  appimage_manager::domain::LaunchSettings default_settings;
  auto on_added = [&](const appimage_manager::domain::AppImageRecord& record) {
    auto settings = launch_settings_repository.load(record.id);
    appimage_manager::domain::LaunchSettings ls = settings.value_or(default_settings);
    appimage_manager::application::generate_desktop(record, ls, applications_dir);
  };
  auto records = scan.execute(config, on_added);

  for (const auto& dir : config.watch_directories) {
    fs::path base(dir);
    if (!fs::is_directory(base)) continue;
    std::cerr << "  " << dir << " contents:";
    int count = 0;
    for (const auto& entry : fs::directory_iterator(base)) {
      std::cerr << " " << entry.path().filename().string();
      ++count;
    }
    if (count == 0)
      std::cerr << " (empty)";
    std::cerr << "\n";
  }

  appimage_manager::daemon::DirectoryWatcher watcher(
    registry, launch_settings_repository, applications_dir, &app);
  watcher.set_config(config);

  QObject* dbus_server = new QObject(&app);
  new appimage_manager::daemon::DBusManagerAdaptor(
    registry, config_repository, launch_settings_repository, applications_dir, &watcher, dbus_server);

  QDBusConnection session = QDBusConnection::sessionBus();
  if (!session.registerObject(QStringLiteral("/org/appimage/Manager1"), dbus_server)) {
    std::cerr << "appimage-manager-daemon: failed to register D-Bus object: "
              << session.lastError().message().toStdString() << "\n";
    return EXIT_FAILURE;
  }
  if (!session.registerService(QStringLiteral("org.appimage.Manager1"))) {
    std::cerr << "appimage-manager-daemon: failed to register D-Bus service: "
              << session.lastError().message().toStdString() << "\n";
    return EXIT_FAILURE;
  }

  std::cout << "appimage-manager-daemon: running (scanned " << records.size() << " AppImage(s))\n";
  return app.exec();
}
