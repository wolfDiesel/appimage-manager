#pragma once

#include "../../domain/repositories/registry_repository.hpp"
#include "../../domain/entities/app_image_record.hpp"
#include <string>

namespace appimage_manager::infrastructure {

class JsonRegistryRepository : public domain::RegistryRepository {
public:
  explicit JsonRegistryRepository(const std::string& config_dir);
  std::vector<domain::AppImageRecord> all() const override;
  std::optional<domain::AppImageRecord> by_path(const std::string& path) const override;
  std::optional<domain::AppImageRecord> by_id(const std::string& id) const override;
  void save(const domain::AppImageRecord& record) override;
  void remove_by_path(const std::string& path) override;

private:
  std::string config_dir_;
  std::string registry_path() const;
  void persist(const std::vector<domain::AppImageRecord>& records) const;
  std::vector<domain::AppImageRecord> load() const;
};

}
