#pragma once

#include "../entities/app_image_record.hpp"
#include <vector>
#include <optional>

namespace appimage_manager::domain {

class RegistryRepository {
public:
  virtual ~RegistryRepository() = default;
  virtual std::vector<AppImageRecord> all() const = 0;
  virtual std::optional<AppImageRecord> by_path(const std::string& path) const = 0;
  virtual std::optional<AppImageRecord> by_id(const std::string& id) const = 0;
  virtual void save(const AppImageRecord& record) = 0;
  virtual void remove_by_path(const std::string& path) = 0;
  virtual void remove(const std::string& id) = 0;
};

}
