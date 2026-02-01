#include "tests.hpp"
#include <domain/entities/app_image_record.hpp>
#include <domain/entities/launch_settings.hpp>
#include <application/generate_desktop.hpp>
#include <cassert>
#include <cstdlib>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace {

int test_desktop_file_path_format() {
  std::string path = appimage_manager::application::desktop_file_path(
      "12345678", "MyApp", "/tmp/apps");
  assert(path.find("/tmp/apps") != std::string::npos);
  assert(path.find("appimagemanager-MyApp-12345678.desktop") != std::string::npos);
  return 0;
}

int test_generate_desktop_creates_file_with_exec_and_env() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-desktop";
  fs::create_directories(tmp);
  std::string apps_dir = tmp.string();
  appimage_manager::domain::AppImageRecord record;
  record.id = "test-id";
  record.path = "/home/user/MyApp.AppImage";
  record.name = "MyApp";
  record.install_type = appimage_manager::domain::InstallType::Downloaded;
  appimage_manager::domain::LaunchSettings settings;
  settings.args = "--foo";
  settings.env = {"MYVAR=value"};
  settings.sandbox = appimage_manager::domain::SandboxMechanism::None;
  appimage_manager::application::generate_desktop(record, settings, apps_dir);
  std::string path = appimage_manager::application::desktop_file_path(record.id, record.name, apps_dir);
  assert(fs::is_regular_file(path));
  std::ifstream f(path);
  std::stringstream buf;
  buf << f.rdbuf();
  std::string content = buf.str();
  assert(content.find("Name=MyApp") != std::string::npos);
  assert(content.find("Exec=") != std::string::npos);
  assert(content.find("MyApp.AppImage") != std::string::npos);
  assert(content.find("--foo") != std::string::npos);
  assert(content.find("Env=MYVAR=value") != std::string::npos);
  fs::remove_all(tmp);
  return 0;
}

int test_generate_desktop_bwrap_wraps_exec() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-desktop-bwrap";
  fs::create_directories(tmp);
  appimage_manager::domain::AppImageRecord record;
  record.id = "bwrap-id";
  record.path = "/opt/App.AppImage";
  record.name = "App";
  appimage_manager::domain::LaunchSettings settings;
  settings.sandbox = appimage_manager::domain::SandboxMechanism::Bwrap;
  appimage_manager::application::generate_desktop(record, settings, tmp.string());
  std::string path = appimage_manager::application::desktop_file_path(record.id, record.name, tmp.string());
  std::ifstream f(path);
  std::stringstream buf;
  buf << f.rdbuf();
  std::string content = buf.str();
  assert(content.find("bwrap") != std::string::npos);
  assert(content.find("/opt/App.AppImage") != std::string::npos);
  fs::remove_all(tmp);
  return 0;
}

int test_generate_desktop_writes_icon_when_provided() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-desktop-icon";
  fs::create_directories(tmp);
  appimage_manager::domain::AppImageRecord record;
  record.id = "icon-id";
  record.path = "/opt/App.AppImage";
  record.name = "App";
  appimage_manager::domain::LaunchSettings settings;
  std::string icon_path = "/home/user/.local/share/appimage-manager/icons/icon-id.png";
  appimage_manager::application::generate_desktop(record, settings, tmp.string(), icon_path);
  std::string path = appimage_manager::application::desktop_file_path(record.id, record.name, tmp.string());
  std::ifstream in(path);
  std::stringstream buf;
  buf << in.rdbuf();
  std::string content = buf.str();
  assert(content.find("Icon=") != std::string::npos);
  assert(content.find("icon-id.png") != std::string::npos);
  fs::remove_all(tmp);
  return 0;
}

int test_remove_desktop_deletes_file() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-remove-desktop";
  fs::create_directories(tmp);
  appimage_manager::domain::AppImageRecord record;
  record.id = "remove-id";
  record.path = "/x.AppImage";
  record.name = "X";
  appimage_manager::domain::LaunchSettings settings;
  appimage_manager::application::generate_desktop(record, settings, tmp.string());
  std::string path = appimage_manager::application::desktop_file_path(record.id, record.name, tmp.string());
  assert(fs::is_regular_file(path));
  appimage_manager::application::remove_desktop(record.id, record.name, tmp.string());
  assert(!fs::is_regular_file(path));
  fs::remove_all(tmp);
  return 0;
}

using test_fn = int (*)();
static const test_fn tests[] = {
  test_desktop_file_path_format,
  test_generate_desktop_creates_file_with_exec_and_env,
  test_generate_desktop_bwrap_wraps_exec,
  test_generate_desktop_writes_icon_when_provided,
  test_remove_desktop_deletes_file,
};
static constexpr std::size_t num_tests = sizeof(tests) / sizeof(tests[0]);

}

std::size_t generate_desktop_test_count() { return num_tests; }

int run_generate_desktop_test(std::size_t i) {
  if (i >= num_tests) return EXIT_FAILURE;
  return tests[i]();
}

int run_generate_desktop_tests() {
  for (std::size_t i = 0; i < num_tests; ++i)
    if (run_generate_desktop_test(i) != 0) return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
