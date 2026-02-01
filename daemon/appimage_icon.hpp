#pragma once

#include <string>

namespace appimage_manager::daemon {

std::string extract_icon_from_appimage(const std::string& appimage_path,
                                       const std::string& icons_dir,
                                       const std::string& record_id);

}
