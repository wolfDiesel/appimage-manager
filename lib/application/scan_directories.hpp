#pragma once

#include <domain/entities/config.hpp>
#include <domain/repositories/registry_repository.hpp>
#include <domain/entities/app_image_record.hpp>
#include <vector>
#include <functional>

namespace appimage_manager::application {

class ScanDirectories {
public:
  using OnAddedCallback = std::function<void(const domain::AppImageRecord&)>;

  explicit ScanDirectories(domain::RegistryRepository& registry);
  std::vector<domain::AppImageRecord> execute(const domain::Config& config,
                                              OnAddedCallback on_added = nullptr,
                                              const std::string& self_path = "");

private:
  domain::RegistryRepository* registry_;
};

}
