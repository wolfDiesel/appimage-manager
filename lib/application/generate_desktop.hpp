#pragma once

#include <domain/entities/app_image_record.hpp>
#include <domain/entities/launch_settings.hpp>
#include <string>

namespace appimage_manager::application {

void generate_desktop(const domain::AppImageRecord& record,
                      const domain::LaunchSettings& settings,
                      const std::string& applications_dir,
                      const std::string& icon_path = "");

void remove_desktop(const std::string& record_id,
                    const std::string& record_name,
                    const std::string& applications_dir);

void remove_icon(const std::string& record_id,
                 const std::string& icons_dir);

std::string desktop_file_path(const std::string& record_id,
                               const std::string& record_name,
                               const std::string& applications_dir);

std::string icon_file_path(const std::string& record_id,
                           const std::string& icons_dir,
                           const std::string& extension = ".png");

}
