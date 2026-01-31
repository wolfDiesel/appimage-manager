#pragma once

#include "../entities/config.hpp"
#include <string>

namespace appimage_manager::domain {

class ConfigRepository {
public:
  virtual ~ConfigRepository() = default;
  virtual Config load() const = 0;
  virtual void save(const Config& config) = 0;
};

}
