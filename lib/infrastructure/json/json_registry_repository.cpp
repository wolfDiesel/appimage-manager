#include "json_registry_repository.hpp"
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace appimage_manager::infrastructure {

namespace {

constexpr const char* registry_filename = "registry.json";

std::string now_iso8601_utc() {
  auto now = std::chrono::system_clock::now();
  auto t = std::chrono::system_clock::to_time_t(now);
  std::tm* tm = std::gmtime(&t);
  if (!tm) return {};
  char buf[32];
  if (std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", tm) == 0)
    return {};
  return std::string(buf);
}

std::string install_type_to_string(domain::InstallType t) {
  switch (t) {
    case domain::InstallType::Downloaded: return "Downloaded";
    case domain::InstallType::GitHub: return "GitHub";
    case domain::InstallType::Direct: return "Direct";
  }
  return "Downloaded";
}

domain::InstallType string_to_install_type(const std::string& s) {
  if (s == "GitHub") return domain::InstallType::GitHub;
  if (s == "Direct") return domain::InstallType::Direct;
  return domain::InstallType::Downloaded;
}

}

JsonRegistryRepository::JsonRegistryRepository(const std::string& config_dir)
  : config_dir_(config_dir) {}

std::string JsonRegistryRepository::registry_path() const {
  return (fs::path(config_dir_) / registry_filename).string();
}

std::vector<domain::AppImageRecord> JsonRegistryRepository::load() const {
  std::vector<domain::AppImageRecord> result;
  std::string path = registry_path();
  if (!fs::is_regular_file(path))
    return result;
  std::ifstream f(path);
  if (!f)
    return result;
  try {
    nlohmann::json j = nlohmann::json::parse(f);
    if (!j.contains("entries") || !j["entries"].is_array())
      return result;
    for (const auto& e : j["entries"]) {
      domain::AppImageRecord record;
      if (e.contains("id") && e["id"].is_string()) record.id = e["id"].get<std::string>();
      if (e.contains("path") && e["path"].is_string()) record.path = e["path"].get<std::string>();
      if (e.contains("name") && e["name"].is_string()) record.name = e["name"].get<std::string>();
      if (e.contains("install_type") && e["install_type"].is_string())
        record.install_type = string_to_install_type(e["install_type"].get<std::string>());
      if (e.contains("added_at") && e["added_at"].is_string())
        record.added_at = e["added_at"].get<std::string>();
      result.push_back(record);
    }
  } catch (...) {
  }
  return result;
}

void JsonRegistryRepository::persist(const std::vector<domain::AppImageRecord>& records) const {
  std::string path = registry_path();
  fs::path dir(path);
  dir.remove_filename();
  if (!dir.empty())
    fs::create_directories(dir);
  nlohmann::json j;
  nlohmann::json arr = nlohmann::json::array();
  for (const auto& r : records) {
    nlohmann::json e;
    e["id"] = r.id;
    e["path"] = r.path;
    e["name"] = r.name;
    e["install_type"] = install_type_to_string(r.install_type);
    e["added_at"] = r.added_at;
    arr.push_back(e);
  }
  j["entries"] = arr;
  std::ofstream f(path);
  if (f)
    f << j.dump(2);
}

std::vector<domain::AppImageRecord> JsonRegistryRepository::all() const {
  return load();
}

std::optional<domain::AppImageRecord> JsonRegistryRepository::by_path(const std::string& path) const {
  auto records = load();
  auto it = std::find_if(records.begin(), records.end(),
    [&path](const domain::AppImageRecord& r) { return r.path == path; });
  if (it == records.end())
    return std::nullopt;
  return *it;
}

std::optional<domain::AppImageRecord> JsonRegistryRepository::by_id(const std::string& id) const {
  auto records = load();
  auto it = std::find_if(records.begin(), records.end(),
    [&id](const domain::AppImageRecord& r) { return r.id == id; });
  if (it == records.end())
    return std::nullopt;
  return *it;
}

void JsonRegistryRepository::save(const domain::AppImageRecord& record) {
  auto records = load();
  auto it = std::find_if(records.begin(), records.end(),
    [&record](const domain::AppImageRecord& r) { return r.path == record.path; });
  domain::AppImageRecord to_save = record;
  if (it != records.end()) {
    if (to_save.added_at.empty())
      to_save.added_at = it->added_at;
    *it = to_save;
  } else {
    if (to_save.added_at.empty())
      to_save.added_at = now_iso8601_utc();
    records.push_back(to_save);
  }
  persist(records);
}

void JsonRegistryRepository::remove_by_path(const std::string& path) {
  auto records = load();
  records.erase(
    std::remove_if(records.begin(), records.end(),
      [&path](const domain::AppImageRecord& r) { return r.path == path; }),
    records.end());
  persist(records);
}

void JsonRegistryRepository::remove(const std::string& id) {
  auto records = load();
  records.erase(
    std::remove_if(records.begin(), records.end(),
      [&id](const domain::AppImageRecord& r) { return r.id == id; }),
    records.end());
  persist(records);
}

}
