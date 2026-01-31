#include "scan_directories.hpp"
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

namespace appimage_manager::application {

namespace {

constexpr std::string_view appimage_suffix = ".AppImage";
constexpr std::string_view part_suffix = ".part";
constexpr std::string_view crdownload_suffix = ".crdownload";

bool is_partial_download(const fs::path& p) {
  std::string name = p.filename().string();
  if (name.size() >= part_suffix.size() &&
      name.compare(name.size() - part_suffix.size(), part_suffix.size(), part_suffix) == 0)
    return true;
  if (name.size() >= crdownload_suffix.size() &&
      name.compare(name.size() - crdownload_suffix.size(), crdownload_suffix.size(), crdownload_suffix) == 0)
    return true;
  return false;
}

bool is_appimage_file(const fs::path& p) {
  if (!fs::is_regular_file(p))
    return false;
  std::string name = p.filename().string();
  if (name.size() < appimage_suffix.size())
    return false;
  std::string suffix(name.end() - static_cast<std::ptrdiff_t>(appimage_suffix.size()), name.end());
  return std::equal(suffix.begin(), suffix.end(), appimage_suffix.begin(), appimage_suffix.end(),
    [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
}

std::string make_id(const std::string& path) {
  return std::to_string(std::hash<std::string>{}(path));
}

}

ScanDirectories::ScanDirectories(domain::RegistryRepository& registry)
  : registry_(&registry) {}

std::vector<domain::AppImageRecord> ScanDirectories::execute(const domain::Config& config,
                                                             OnAddedCallback on_added,
                                                             const std::string& self_path) {
  std::vector<domain::AppImageRecord> result;
  for (const auto& dir : config.watch_directories) {
    fs::path base(dir);
    if (!fs::is_directory(base))
      continue;
    for (const auto& entry : fs::directory_iterator(base)) {
      const auto& p = entry.path();
      if (is_partial_download(p))
        continue;
      if (!is_appimage_file(p))
        continue;
      std::string path = p.string();
      if (!self_path.empty()) {
        std::error_code ec;
        if (fs::equivalent(p, fs::path(self_path), ec))
          continue;
      }
      auto existing = registry_->by_path(path);
      if (existing) {
        result.push_back(*existing);
        continue;
      }
      domain::AppImageRecord record;
      record.id = make_id(path);
      record.path = path;
      record.name = p.stem().string();
      record.install_type = domain::InstallType::Downloaded;
      std::error_code perm_ec;
      fs::permissions(p, fs::perms::owner_exec, fs::perm_options::add, perm_ec);
      registry_->save(record);
      result.push_back(record);
      if (on_added)
        on_added(record);
    }
  }
  return result;
}

}
