#include "tests.hpp"
#include <domain/entities/launch_settings.hpp>
#include <infrastructure/json/json_launch_settings_repository.hpp>
#include <cassert>
#include <cstdlib>
#include <cstddef>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {

int test_launch_settings_round_trip() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-launch";
  fs::create_directories(tmp);
  appimage_manager::infrastructure::JsonLaunchSettingsRepository repo(tmp.string());
  appimage_manager::domain::LaunchSettings ls;
  ls.args = "--debug";
  ls.env = {"A=1", "B=2"};
  ls.sandbox = appimage_manager::domain::SandboxMechanism::Firejail;
  repo.save("app-id-1", ls);
  auto loaded = repo.load("app-id-1");
  assert(loaded);
  assert(loaded->args == ls.args);
  assert(loaded->env == ls.env);
  assert(loaded->sandbox == ls.sandbox);
  auto all = repo.load_all();
  assert(all.size() == 1u);
  assert(all.at("app-id-1").args == ls.args);
  fs::remove_all(tmp);
  return 0;
}

int test_launch_settings_missing_returns_nullopt() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-launch-missing";
  fs::create_directories(tmp);
  appimage_manager::infrastructure::JsonLaunchSettingsRepository repo(tmp.string());
  assert(!repo.load("nonexistent"));
  assert(repo.load_all().empty());
  fs::remove_all(tmp);
  return 0;
}

int test_launch_settings_invalid_json_returns_empty() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-launch-invalid";
  fs::create_directories(tmp);
  std::ofstream((tmp / "launch_settings.json").string()) << "{ broken ";
  appimage_manager::infrastructure::JsonLaunchSettingsRepository repo(tmp.string());
  assert(repo.load_all().empty());
  fs::remove_all(tmp);
  return 0;
}

using test_fn = int (*)();
static const test_fn tests[] = {
  test_launch_settings_round_trip,
  test_launch_settings_missing_returns_nullopt,
  test_launch_settings_invalid_json_returns_empty,
};
static constexpr std::size_t num_tests = sizeof(tests) / sizeof(tests[0]);

}

std::size_t launch_settings_repository_test_count() { return num_tests; }

int run_launch_settings_repository_test(std::size_t i) {
  if (i >= num_tests) return EXIT_FAILURE;
  return tests[i]();
}

int run_launch_settings_repository_tests() {
  for (std::size_t i = 0; i < num_tests; ++i)
    if (run_launch_settings_repository_test(i) != 0) return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
