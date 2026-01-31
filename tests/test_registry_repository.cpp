#include "tests.hpp"
#include <domain/entities/app_image_record.hpp>
#include <infrastructure/json/json_registry_repository.hpp>
#include <cassert>
#include <cstdlib>
#include <cstddef>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {

int test_registry_round_trip() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-registry";
  fs::create_directories(tmp);
  appimage_manager::infrastructure::JsonRegistryRepository repo(tmp.string());
  appimage_manager::domain::AppImageRecord r;
  r.id = "id1";
  r.path = "/opt/App.AppImage";
  r.name = "App";
  r.install_type = appimage_manager::domain::InstallType::Direct;
  repo.save(r);
  auto all = repo.all();
  assert(all.size() == 1u);
  assert(all[0].id == r.id && all[0].path == r.path && all[0].name == r.name);
  assert(all[0].install_type == appimage_manager::domain::InstallType::Direct);
  assert(!all[0].added_at.empty());
  auto by_p = repo.by_path(r.path);
  assert(by_p && by_p->id == r.id);
  auto by_i = repo.by_id(r.id);
  assert(by_i && by_i->path == r.path);
  fs::remove_all(tmp);
  return 0;
}

int test_registry_update_existing_by_path() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-registry-update";
  fs::create_directories(tmp);
  appimage_manager::infrastructure::JsonRegistryRepository repo(tmp.string());
  appimage_manager::domain::AppImageRecord r;
  r.id = "id1";
  r.path = "/opt/App.AppImage";
  r.name = "App";
  r.added_at = "2020-01-01T12:00:00Z";
  repo.save(r);
  r.name = "AppRenamed";
  repo.save(r);
  auto all = repo.all();
  assert(all.size() == 1u);
  assert(all[0].name == "AppRenamed");
  assert(all[0].added_at == "2020-01-01T12:00:00Z");
  fs::remove_all(tmp);
  return 0;
}

int test_registry_remove_by_path() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-registry-remove";
  fs::create_directories(tmp);
  appimage_manager::infrastructure::JsonRegistryRepository repo(tmp.string());
  appimage_manager::domain::AppImageRecord r;
  r.id = "id1";
  r.path = "/opt/App.AppImage";
  r.name = "App";
  repo.save(r);
  repo.remove_by_path(r.path);
  assert(repo.all().empty());
  assert(!repo.by_path(r.path));
  assert(!repo.by_id(r.id));
  fs::remove_all(tmp);
  return 0;
}

int test_registry_remove_by_id() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-registry-remove-id";
  fs::create_directories(tmp);
  appimage_manager::infrastructure::JsonRegistryRepository repo(tmp.string());
  appimage_manager::domain::AppImageRecord r;
  r.id = "id-to-remove";
  r.path = "/opt/Some.AppImage";
  r.name = "Some";
  repo.save(r);
  repo.remove("id-to-remove");
  assert(repo.all().empty());
  assert(!repo.by_path(r.path));
  assert(!repo.by_id(r.id));
  fs::remove_all(tmp);
  return 0;
}

int test_registry_empty_dir_returns_empty() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-registry-empty";
  fs::create_directories(tmp);
  appimage_manager::infrastructure::JsonRegistryRepository repo(tmp.string());
  assert(repo.all().empty());
  assert(!repo.by_path("/any"));
  assert(!repo.by_id("any"));
  fs::remove_all(tmp);
  return 0;
}

int test_registry_invalid_json_returns_empty() {
  fs::path tmp = fs::temp_directory_path() / "appimage-manager-test-registry-invalid";
  fs::create_directories(tmp);
  std::ofstream((tmp / "registry.json").string()) << "not json at all";
  appimage_manager::infrastructure::JsonRegistryRepository repo(tmp.string());
  assert(repo.all().empty());
  fs::remove_all(tmp);
  return 0;
}

using test_fn = int (*)();
static const test_fn tests[] = {
  test_registry_round_trip,
  test_registry_update_existing_by_path,
  test_registry_remove_by_path,
  test_registry_remove_by_id,
  test_registry_empty_dir_returns_empty,
  test_registry_invalid_json_returns_empty,
};
static constexpr std::size_t num_tests = sizeof(tests) / sizeof(tests[0]);

}

std::size_t registry_repository_test_count() { return num_tests; }

int run_registry_repository_test(std::size_t i) {
  if (i >= num_tests) return EXIT_FAILURE;
  return tests[i]();
}

int run_registry_repository_tests() {
  for (std::size_t i = 0; i < num_tests; ++i)
    if (run_registry_repository_test(i) != 0) return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
