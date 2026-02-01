find_package(nlohmann_json REQUIRED)

add_library(appimage-manager-core STATIC
  domain/entities/install_type.hpp
  domain/entities/app_image_record.hpp
  domain/entities/config.hpp
  domain/entities/launch_settings.hpp
  domain/repositories/registry_repository.hpp
  domain/repositories/config_repository.hpp
  domain/repositories/launch_settings_repository.hpp
  application/scan_directories.hpp
  application/scan_directories.cpp
  application/extract_icon.hpp
  application/extract_icon.cpp
  application/generate_desktop.hpp
  application/generate_desktop.cpp
  infrastructure/json/json_config_repository.hpp
  infrastructure/json/json_config_repository.cpp
  infrastructure/json/json_registry_repository.hpp
  infrastructure/json/json_registry_repository.cpp
  infrastructure/json/json_launch_settings_repository.hpp
  infrastructure/json/json_launch_settings_repository.cpp
)

target_include_directories(appimage-manager-core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(appimage-manager-core PUBLIC nlohmann_json::nlohmann_json)
