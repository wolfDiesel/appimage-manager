#pragma once

#include "../../domain/repositories/launch_settings_repository.hpp"
#include "../../domain/entities/launch_settings.hpp"
#include <string>

namespace appimage_manager::infrastructure {

class JsonLaunchSettingsRepository : public domain::LaunchSettingsRepository {
public:
  explicit JsonLaunchSettingsRepository(const std::string& config_dir);
  std::optional<domain::LaunchSettings> load(const std::string& app_id) const override;
  void save(const std::string& app_id, const domain::LaunchSettings& settings) override;
  std::unordered_map<std::string, domain::LaunchSettings> load_all() const override;

private:
  std::string config_dir_;
  std::string launch_settings_path() const;
};

}
