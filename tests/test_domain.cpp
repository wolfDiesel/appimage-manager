#include "tests.hpp"
#include <domain/entities/install_type.hpp>
#include <domain/entities/app_image_record.hpp>
#include <domain/entities/launch_settings.hpp>
#include <cassert>
#include <cstdlib>
#include <cstddef>

namespace {

int test_install_type_values() {
  auto t = appimage_manager::domain::InstallType::Downloaded;
  assert(t == appimage_manager::domain::InstallType::Downloaded);
  t = appimage_manager::domain::InstallType::GitHub;
  assert(t == appimage_manager::domain::InstallType::GitHub);
  t = appimage_manager::domain::InstallType::Direct;
  assert(t == appimage_manager::domain::InstallType::Direct);
  return 0;
}

int test_app_image_record_fields() {
  appimage_manager::domain::AppImageRecord r;
  r.id = "123";
  r.path = "/tmp/MyApp.AppImage";
  r.name = "MyApp";
  r.install_type = appimage_manager::domain::InstallType::GitHub;
  assert(r.id == "123");
  assert(r.path == "/tmp/MyApp.AppImage");
  assert(r.name == "MyApp");
  assert(r.install_type == appimage_manager::domain::InstallType::GitHub);
  return 0;
}

int test_launch_settings_and_sandbox() {
  appimage_manager::domain::LaunchSettings ls;
  assert(ls.sandbox == appimage_manager::domain::SandboxMechanism::None);
  ls.args = "--verbose";
  ls.env = {"FOO=1", "BAR=2"};
  ls.sandbox = appimage_manager::domain::SandboxMechanism::Bwrap;
  assert(ls.args == "--verbose");
  assert(ls.env.size() == 2u);
  assert(ls.sandbox == appimage_manager::domain::SandboxMechanism::Bwrap);
  return 0;
}

using test_fn = int (*)();
static const test_fn tests[] = {
  test_install_type_values,
  test_app_image_record_fields,
  test_launch_settings_and_sandbox,
};
static constexpr std::size_t num_tests = sizeof(tests) / sizeof(tests[0]);

}

std::size_t domain_test_count() { return num_tests; }

int run_domain_test(std::size_t i) {
  if (i >= num_tests) return EXIT_FAILURE;
  return tests[i]();
}

int run_domain_tests() {
  for (std::size_t i = 0; i < num_tests; ++i)
    if (run_domain_test(i) != 0) return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
