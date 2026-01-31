#pragma once

#include <cstddef>

int run_domain_tests();
int run_domain_test(std::size_t i);
std::size_t domain_test_count();

int run_config_repository_tests();
int run_config_repository_test(std::size_t i);
std::size_t config_repository_test_count();

int run_registry_repository_tests();
int run_registry_repository_test(std::size_t i);
std::size_t registry_repository_test_count();

int run_launch_settings_repository_tests();
int run_launch_settings_repository_test(std::size_t i);
std::size_t launch_settings_repository_test_count();

int run_scan_directories_tests();
int run_scan_directories_test(std::size_t i);
std::size_t scan_directories_test_count();

int run_generate_desktop_tests();
int run_generate_desktop_test(std::size_t i);
std::size_t generate_desktop_test_count();

int run_dbus_getallrecords_tests();
int run_dbus_getallrecords_test(std::size_t i);
std::size_t dbus_getallrecords_test_count();
