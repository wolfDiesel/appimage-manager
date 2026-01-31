#pragma once

#include "../entities/launch_settings.hpp"
#include <string>
#include <optional>
#include <unordered_map>

namespace appimage_manager::domain {

class LaunchSettingsRepository {
public:
  virtual ~LaunchSettingsRepository() = default;
  virtual std::optional<LaunchSettings> load(const std::string& app_id) const = 0;
  virtual void save(const std::string& app_id, const LaunchSettings& settings) = 0;
  virtual std::unordered_map<std::string, LaunchSettings> load_all() const = 0;
};

}
