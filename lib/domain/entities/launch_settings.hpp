#pragma once

#include <string>
#include <vector>

namespace appimage_manager::domain {

enum class SandboxMechanism {
  None,
  Bwrap,
  Firejail,
};

struct LaunchSettings {
  std::string args;
  std::vector<std::string> env;
  SandboxMechanism sandbox{SandboxMechanism::None};
};

}
