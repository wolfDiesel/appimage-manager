#pragma once

#include "install_type.hpp"
#include <string>

namespace appimage_manager::domain {

struct AppImageRecord {
  std::string id;
  std::string path;
  std::string name;
  InstallType install_type{InstallType::Downloaded};
  std::string added_at;
};

}
