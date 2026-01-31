#include "directory_watcher.hpp"
#include <domain/entities/config.hpp>
#include <domain/entities/launch_settings.hpp>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cctype>

namespace fs = std::filesystem;

namespace appimage_manager::daemon {

namespace {

constexpr std::string_view appimage_suffix = ".AppImage";

bool is_appimage_path(const std::string& path) {
  fs::path p(path);
  if (!fs::is_regular_file(p))
    return false;
  std::string name = p.filename().string();
  if (name.size() < appimage_suffix.size())
    return false;
  std::string suffix(name.end() - static_cast<std::ptrdiff_t>(appimage_suffix.size()), name.end());
  return std::equal(suffix.begin(), suffix.end(), appimage_suffix.begin(), appimage_suffix.end(),
    [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
}

}

DirectoryWatcher::DirectoryWatcher(domain::RegistryRepository& registry,
                                 domain::LaunchSettingsRepository& launch_settings_repository,
                                 const std::string& applications_dir,
                                 QObject* parent)
  : QObject(parent)
  , registry_(&registry)
  , launch_settings_repository_(&launch_settings_repository)
  , applications_dir_(applications_dir)
  , scan_(registry) {
  connect(&watcher_, &QFileSystemWatcher::directoryChanged, this, &DirectoryWatcher::on_directory_changed);
}

void DirectoryWatcher::set_config(const domain::Config& config) {
  for (const QString& path : watcher_.directories())
    watcher_.removePath(path);
  config_ = config;
  for (const auto& dir : config_.watch_directories) {
    if (fs::is_directory(fs::path(dir)))
      watcher_.addPath(QString::fromStdString(dir));
  }
}

void DirectoryWatcher::trigger_rescan() {
  for (const auto& dir : config_.watch_directories)
    scan_directory(dir);
  Q_EMIT records_changed();
}

void DirectoryWatcher::on_directory_changed(const QString& path) {
  std::cerr << "appimage-manager-daemon: inotify: directory changed " << path.toStdString() << "\n";
  scan_directory(path.toStdString());
  remove_stale_records_for_directory(path.toStdString());
  Q_EMIT records_changed();
}

void DirectoryWatcher::scan_directory(const std::string& dir_path) {
  domain::Config single;
  single.watch_directories.push_back(dir_path);
  domain::LaunchSettings default_settings;
  auto on_added = [this, &default_settings](const domain::AppImageRecord& record) {
    domain::LaunchSettings settings = default_settings;
    auto loaded = launch_settings_repository_->load(record.id);
    if (loaded)
      settings = *loaded;
    application::generate_desktop(record, settings, applications_dir_);
  };
  scan_.execute(single, on_added);
}

void DirectoryWatcher::remove_stale_records_for_directory(const std::string& dir_path) {
  fs::path base(dir_path);
  if (!fs::is_directory(base))
    return;
  std::vector<std::string> current_paths;
  for (const auto& entry : fs::directory_iterator(base)) {
    std::string path = entry.path().string();
    if (is_appimage_path(path))
      current_paths.push_back(path);
  }
  auto all = registry_->all();
  for (const auto& record : all) {
    fs::path rec_path(record.path);
    if (rec_path.parent_path() != base)
      continue;
    bool still_exists = std::find(current_paths.begin(), current_paths.end(), record.path) != current_paths.end();
    if (!still_exists) {
      registry_->remove_by_path(record.path);
      application::remove_desktop(record.id, applications_dir_);
    }
  }
}

}
