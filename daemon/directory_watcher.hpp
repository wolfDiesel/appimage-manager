#pragma once

#include <domain/entities/config.hpp>
#include <domain/repositories/registry_repository.hpp>
#include <domain/repositories/launch_settings_repository.hpp>
#include <application/scan_directories.hpp>
#include <application/generate_desktop.hpp>
#include <QObject>
#include <QFileSystemWatcher>
#include <QStringList>
#include <string>

namespace appimage_manager::daemon {

class DirectoryWatcher : public QObject {
  Q_OBJECT
public:
  DirectoryWatcher(domain::RegistryRepository& registry,
                  domain::LaunchSettingsRepository& launch_settings_repository,
                  const std::string& applications_dir,
                  QObject* parent = nullptr);

  void set_config(const domain::Config& config);
  void trigger_rescan();

signals:
  void records_changed();

private Q_SLOTS:
  void on_directory_changed(const QString& path);

private:
  void scan_directory(const std::string& dir_path);
  void remove_stale_records_for_directory(const std::string& dir_path);

  domain::RegistryRepository* registry_;
  domain::LaunchSettingsRepository* launch_settings_repository_;
  std::string applications_dir_;
  domain::Config config_;
  QFileSystemWatcher watcher_;
  application::ScanDirectories scan_;
};

}
