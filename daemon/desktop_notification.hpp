#pragma once

#include <string>

namespace appimage_manager::daemon {

void notify_appimage_processed(const std::string& filename,
                               const std::string& dir_path);

}
