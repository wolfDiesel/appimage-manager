#pragma once

#include <array>
#include <string_view>

namespace appimage_manager::daemon {

constexpr std::array<std::pair<std::string_view, std::string_view>, 14> icon_search_paths = {{
  {"usr/share/icons/hicolor/512x512/apps/", ".png"},
  {"usr/share/icons/hicolor/256x256/apps/", ".png"},
  {"usr/share/icons/hicolor/128x128/apps/", ".png"},
  {"usr/share/icons/hicolor/96x96/apps/", ".png"},
  {"usr/share/icons/hicolor/scalable/apps/", ".svg"},
  {"usr/share/pixmaps/", ".png"},
  {"usr/share/pixmaps/", ".svg"},
  {"resources/icons/", ".png"},
  {"resources/icons/", ".svg"},
  {"usr/share/icons/hicolor/64x64/apps/", ".png"},
  {"usr/share/icons/hicolor/48x48/apps/", ".png"},
  {"usr/share/icons/hicolor/32x32/apps/", ".png"},
  {"", ".png"},
  {"", ".svg"},
}};

}
