#include "tests.hpp"
#include <domain/entities/config.hpp>
#include <application/scan_directories.hpp>
#include <infrastructure/json/json_registry_repository.hpp>
#include <cassert>
#include <cstdlib>
#include <cstddef>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {

int test_scan_directories_finds_appimage_and_saves_to_registry() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-scan";
  fs::create_directories(tmp);
  std::string app_path = (tmp / "MyTool.AppImage").string();
  std::ofstream(app_path).put('x');
  appimage_manager::infrastructure::JsonRegistryRepository registry(tmp.string());
  appimage_manager::application::ScanDirectories scan(registry);
  appimage_manager::domain::Config config;
  config.watch_directories.push_back(tmp.string());
  std::vector<appimage_manager::domain::AppImageRecord> added;
  auto records = scan.execute(config, [&added](const appimage_manager::domain::AppImageRecord& r) { added.push_back(r); });
  assert(records.size() == 1u);
  assert(records[0].path == app_path);
  assert(records[0].name == "MyTool");
  assert(records[0].install_type == appimage_manager::domain::InstallType::Downloaded);
  assert(added.size() == 1u);
  auto all = registry.all();
  assert(all.size() == 1u);
  assert(all[0].path == app_path);
  fs::remove_all(tmp);
  return 0;
}

int test_scan_directories_ignores_part_and_crdownload() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-scan-ignore";
  fs::create_directories(tmp);
  std::ofstream((tmp / "real.AppImage").string()).put('x');
  std::ofstream((tmp / "downloading.AppImage.part").string()).put('x');
  std::ofstream((tmp / "chrome.AppImage.crdownload").string()).put('x');
  appimage_manager::infrastructure::JsonRegistryRepository registry(tmp.string());
  appimage_manager::application::ScanDirectories scan(registry);
  appimage_manager::domain::Config config;
  config.watch_directories.push_back(tmp.string());
  auto records = scan.execute(config, nullptr);
  assert(records.size() == 1u);
  assert(records[0].name == "real");
  fs::remove_all(tmp);
  return 0;
}

int test_scan_directories_skips_self_path() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-scan-self";
  fs::create_directories(tmp);
  std::string self_path = (tmp / "self.AppImage").string();
  std::ofstream(self_path).put('x');
  std::string other_path = (tmp / "other.AppImage").string();
  std::ofstream(other_path).put('x');
  appimage_manager::infrastructure::JsonRegistryRepository registry(tmp.string());
  appimage_manager::application::ScanDirectories scan(registry);
  appimage_manager::domain::Config config;
  config.watch_directories.push_back(tmp.string());
  auto records = scan.execute(config, nullptr, self_path);
  assert(records.size() == 1u);
  assert(records[0].path == other_path);
  assert(records[0].name == "other");
  auto all = registry.all();
  assert(all.size() == 1u);
  assert(all[0].path == other_path);
  fs::remove_all(tmp);
  return 0;
}

using test_fn = int (*)();
static const test_fn tests[] = {
  test_scan_directories_finds_appimage_and_saves_to_registry,
  test_scan_directories_ignores_part_and_crdownload,
  test_scan_directories_skips_self_path,
};
static constexpr std::size_t num_tests = sizeof(tests) / sizeof(tests[0]);

}

std::size_t scan_directories_test_count() { return num_tests; }

int run_scan_directories_test(std::size_t i) {
  if (i >= num_tests) return EXIT_FAILURE;
  return tests[i]();
}

int run_scan_directories_tests() {
  for (std::size_t i = 0; i < num_tests; ++i)
    if (run_scan_directories_test(i) != 0) return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
