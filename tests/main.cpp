#include "tests.hpp"
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <string>
#include <QCoreApplication>

static int run_one_test(const char* group, std::size_t index) {
  if (strcmp(group, "domain") == 0) return run_domain_test(index);
  if (strcmp(group, "config_repository") == 0) return run_config_repository_test(index);
  if (strcmp(group, "registry_repository") == 0) return run_registry_repository_test(index);
  if (strcmp(group, "launch_settings_repository") == 0) return run_launch_settings_repository_test(index);
  if (strcmp(group, "scan_directories") == 0) return run_scan_directories_test(index);
  if (strcmp(group, "generate_desktop") == 0) return run_generate_desktop_test(index);
  if (strcmp(group, "dbus_getallrecords") == 0) return run_dbus_getallrecords_test(index);
  return EXIT_FAILURE;
}

int main(int argc, char* argv[]) {
  if (argc >= 3) {
    std::size_t index = static_cast<std::size_t>(std::atoi(argv[2]));
    return run_one_test(argv[1], index) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  if (argc >= 2) {
    if (strcmp(argv[1], "domain") == 0) return run_domain_tests() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    if (strcmp(argv[1], "config_repository") == 0) return run_config_repository_tests() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    if (strcmp(argv[1], "registry_repository") == 0) return run_registry_repository_tests() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    if (strcmp(argv[1], "launch_settings_repository") == 0) return run_launch_settings_repository_tests() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    if (strcmp(argv[1], "scan_directories") == 0) return run_scan_directories_tests() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    if (strcmp(argv[1], "generate_desktop") == 0) return run_generate_desktop_tests() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    if (strcmp(argv[1], "dbus_getallrecords") == 0) {
      QCoreApplication app(argc, argv);
      return run_dbus_getallrecords_tests() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    return EXIT_FAILURE;
  }
  if (run_domain_tests() != 0) return EXIT_FAILURE;
  if (run_config_repository_tests() != 0) return EXIT_FAILURE;
  if (run_registry_repository_tests() != 0) return EXIT_FAILURE;
  if (run_launch_settings_repository_tests() != 0) return EXIT_FAILURE;
  if (run_scan_directories_tests() != 0) return EXIT_FAILURE;
  if (run_generate_desktop_tests() != 0) return EXIT_FAILURE;
  {
    int argc = 1;
    char* argv0 = argv[0];
    char* argv[] = { argv0, nullptr };
    QCoreApplication app(argc, argv);
    if (run_dbus_getallrecords_tests() != 0) return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
