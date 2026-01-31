#include "json_config_repository.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace appimage_manager::infrastructure {

namespace {

constexpr const char* config_filename = "config.json";

}

JsonConfigRepository::JsonConfigRepository(const std::string& config_dir)
  : config_dir_(config_dir) {}

std::string JsonConfigRepository::config_path() const {
  return (fs::path(config_dir_) / config_filename).string();
}

domain::Config JsonConfigRepository::load() const {
  domain::Config result;
  std::string path = config_path();
  if (!fs::is_regular_file(path))
    return result;
  std::ifstream f(path);
  if (!f)
    return result;
  try {
    nlohmann::json j = nlohmann::json::parse(f);
    if (j.contains("watch_directories") && j["watch_directories"].is_array()) {
      for (const auto& item : j["watch_directories"])
        if (item.is_string())
          result.watch_directories.push_back(item.get<std::string>());
    }
  } catch (...) {
  }
  return result;
}

void JsonConfigRepository::save(const domain::Config& config) {
  std::string path = config_path();
  fs::path dir(path);
  dir.remove_filename();
  if (!dir.empty())
    fs::create_directories(dir);
  nlohmann::json j;
  j["watch_directories"] = config.watch_directories;
  std::ofstream f(path);
  if (f)
    f << j.dump(2);
}

}
