#include "json_launch_settings_repository.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace appimage_manager::infrastructure {

namespace {

constexpr const char* launch_settings_filename = "launch_settings.json";

std::string sandbox_to_string(domain::SandboxMechanism s) {
  switch (s) {
    case domain::SandboxMechanism::Bwrap: return "bwrap";
    case domain::SandboxMechanism::Firejail: return "firejail";
    default: return "none";
  }
}

domain::SandboxMechanism string_to_sandbox(const std::string& s) {
  if (s == "bwrap") return domain::SandboxMechanism::Bwrap;
  if (s == "firejail") return domain::SandboxMechanism::Firejail;
  return domain::SandboxMechanism::None;
}

domain::LaunchSettings launch_settings_from_json(const nlohmann::json& j) {
  domain::LaunchSettings ls;
  if (j.contains("args") && j["args"].is_string())
    ls.args = j["args"].get<std::string>();
  if (j.contains("env") && j["env"].is_array()) {
    for (const auto& e : j["env"])
      if (e.is_string())
        ls.env.push_back(e.get<std::string>());
  }
  if (j.contains("sandbox") && j["sandbox"].is_string())
    ls.sandbox = string_to_sandbox(j["sandbox"].get<std::string>());
  return ls;
}

nlohmann::json launch_settings_to_json(const domain::LaunchSettings& ls) {
  nlohmann::json j;
  j["args"] = ls.args;
  j["env"] = ls.env;
  j["sandbox"] = sandbox_to_string(ls.sandbox);
  return j;
}

}

JsonLaunchSettingsRepository::JsonLaunchSettingsRepository(const std::string& config_dir)
  : config_dir_(config_dir) {}

std::string JsonLaunchSettingsRepository::launch_settings_path() const {
  return (fs::path(config_dir_) / launch_settings_filename).string();
}

std::optional<domain::LaunchSettings> JsonLaunchSettingsRepository::load(const std::string& app_id) const {
  auto all = load_all();
  auto it = all.find(app_id);
  if (it == all.end())
    return std::nullopt;
  return it->second;
}

std::unordered_map<std::string, domain::LaunchSettings> JsonLaunchSettingsRepository::load_all() const {
  std::unordered_map<std::string, domain::LaunchSettings> result;
  std::string path = launch_settings_path();
  if (!fs::is_regular_file(path))
    return result;
  std::ifstream f(path);
  if (!f)
    return result;
  try {
    nlohmann::json j = nlohmann::json::parse(f);
    if (!j.contains("settings") || !j["settings"].is_object())
      return result;
    for (auto it = j["settings"].begin(); it != j["settings"].end(); ++it)
      result[it.key()] = launch_settings_from_json(it.value());
  } catch (...) {
  }
  return result;
}

void JsonLaunchSettingsRepository::save(const std::string& app_id, const domain::LaunchSettings& settings) {
  auto all = load_all();
  all[app_id] = settings;
  std::string path = launch_settings_path();
  fs::path dir(path);
  dir.remove_filename();
  if (!dir.empty())
    fs::create_directories(dir);
  nlohmann::json j;
  nlohmann::json settings_obj;
  for (const auto& [id, ls] : all)
    settings_obj[id] = launch_settings_to_json(ls);
  j["settings"] = settings_obj;
  std::ofstream f(path);
  if (f)
    f << j.dump(2);
}

}
