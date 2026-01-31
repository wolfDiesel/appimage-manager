#include "tests.hpp"
#include <domain/entities/config.hpp>
#include <infrastructure/json/json_config_repository.hpp>
#include <cassert>
#include <cstdlib>
#include <cstddef>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {

int test_config_round_trip() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-config";
  fs::create_directories(tmp);
  std::string config_dir = tmp.string();
  appimage_manager::infrastructure::JsonConfigRepository repo(config_dir);
  appimage_manager::domain::Config config;
  config.watch_directories.push_back("/tmp");
  config.watch_directories.push_back("/home/user/Apps");
  repo.save(config);
  auto loaded = repo.load();
  assert(loaded.watch_directories.size() == config.watch_directories.size());
  assert(loaded.watch_directories[0] == "/tmp");
  assert(loaded.watch_directories[1] == "/home/user/Apps");
  fs::remove_all(tmp);
  return 0;
}

int test_empty_dir_returns_empty_config() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-empty";
  fs::create_directories(tmp);
  appimage_manager::infrastructure::JsonConfigRepository repo(tmp.string());
  auto loaded = repo.load();
  assert(loaded.watch_directories.empty());
  fs::remove_all(tmp);
  return 0;
}

int test_config_invalid_json_returns_empty() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-invalid-config";
  fs::create_directories(tmp);
  std::ofstream((tmp / "config.json").string()) << "{ invalid json ";
  appimage_manager::infrastructure::JsonConfigRepository repo(tmp.string());
  auto loaded = repo.load();
  assert(loaded.watch_directories.empty());
  fs::remove_all(tmp);
  return 0;
}

using test_fn = int (*)();
static const test_fn tests[] = {
  test_config_round_trip,
  test_empty_dir_returns_empty_config,
  test_config_invalid_json_returns_empty,
};
static constexpr std::size_t num_tests = sizeof(tests) / sizeof(tests[0]);

}

std::size_t config_repository_test_count() { return num_tests; }

int run_config_repository_test(std::size_t i) {
  if (i >= num_tests) return EXIT_FAILURE;
  return tests[i]();
}

int run_config_repository_tests() {
  for (std::size_t i = 0; i < num_tests; ++i)
    if (run_config_repository_test(i) != 0) return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
