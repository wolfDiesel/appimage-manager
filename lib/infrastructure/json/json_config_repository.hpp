#pragma once

#include "../../domain/repositories/config_repository.hpp"
#include "../../domain/entities/config.hpp"
#include <string>

namespace appimage_manager::infrastructure {

class JsonConfigRepository : public domain::ConfigRepository {
public:
  explicit JsonConfigRepository(const std::string& config_dir);
  domain::Config load() const override;
  void save(const domain::Config& config) override;

private:
  std::string config_dir_;
  std::string config_path() const;
};

}
