#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace appimage_manager::application {

std::optional<std::uint64_t> get_appimage_squashfs_offset(const std::string& appimage_path);

}
