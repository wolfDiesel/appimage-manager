#pragma once

#include <string>
#include <vector>

namespace appimage_manager::domain {

struct Config {
  std::vector<std::string> watch_directories;
};

}
