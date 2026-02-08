cmake_minimum_required(VERSION 3.16)
project(AppimageManager VERSION 0.2.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets DBus Network)

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_APPIMAGE "Build AppImage at build stage" OFF)
if(BUILD_TESTS)
  enable_testing()
endif()

add_subdirectory(lib)
add_subdirectory(daemon)
add_subdirectory(gui)

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()

add_custom_target(appimage
  COMMAND chmod +x ${CMAKE_SOURCE_DIR}/packaging/appimage/build_appimage.sh
  COMMAND ${CMAKE_SOURCE_DIR}/packaging/appimage/build_appimage.sh ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/appimage-manager.AppImage
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Building AppImage"
  DEPENDS appimage-manager-daemon appimage-manager-gui
  VERBATIM
)
if(BUILD_APPIMAGE)
  set_target_properties(appimage PROPERTIES EXCLUDE_FROM_ALL FALSE)
endif()
