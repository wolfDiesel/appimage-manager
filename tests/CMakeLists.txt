add_executable(appimage-manager-tests
  main.cpp
  test_domain.cpp
  test_config_repository.cpp
  test_registry_repository.cpp
  test_launch_settings_repository.cpp
  test_scan_directories.cpp
  test_generate_desktop.cpp
  test_dbus_getallrecords.cpp
)
target_include_directories(appimage-manager-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(appimage-manager-tests PRIVATE appimage-manager-core Qt6::Core Qt6::DBus)
add_test(NAME domain_install_type COMMAND appimage-manager-tests domain 0)
add_test(NAME domain_app_image_record COMMAND appimage-manager-tests domain 1)
add_test(NAME domain_launch_settings COMMAND appimage-manager-tests domain 2)
add_test(NAME config_repository_round_trip COMMAND appimage-manager-tests config_repository 0)
add_test(NAME config_repository_empty_dir COMMAND appimage-manager-tests config_repository 1)
add_test(NAME config_repository_invalid_json COMMAND appimage-manager-tests config_repository 2)
add_test(NAME registry_repository_round_trip COMMAND appimage-manager-tests registry_repository 0)
add_test(NAME registry_repository_update_by_path COMMAND appimage-manager-tests registry_repository 1)
add_test(NAME registry_repository_remove_by_path COMMAND appimage-manager-tests registry_repository 2)
add_test(NAME registry_repository_empty_dir COMMAND appimage-manager-tests registry_repository 3)
add_test(NAME registry_repository_invalid_json COMMAND appimage-manager-tests registry_repository 4)
add_test(NAME launch_settings_repository_round_trip COMMAND appimage-manager-tests launch_settings_repository 0)
add_test(NAME launch_settings_repository_missing_nullopt COMMAND appimage-manager-tests launch_settings_repository 1)
add_test(NAME launch_settings_repository_invalid_json COMMAND appimage-manager-tests launch_settings_repository 2)
add_test(NAME scan_directories_finds_appimage COMMAND appimage-manager-tests scan_directories 0)
add_test(NAME scan_directories_ignores_part_crdownload COMMAND appimage-manager-tests scan_directories 1)
add_test(NAME generate_desktop_file_path_format COMMAND appimage-manager-tests generate_desktop 0)
add_test(NAME generate_desktop_exec_and_env COMMAND appimage-manager-tests generate_desktop 1)
add_test(NAME generate_desktop_bwrap_wraps COMMAND appimage-manager-tests generate_desktop 2)
add_test(NAME generate_desktop_remove_deletes_file COMMAND appimage-manager-tests generate_desktop 3)
add_test(NAME dbus_getallrecords_record_round_trip COMMAND appimage-manager-tests dbus_getallrecords 0)
add_test(NAME dbus_getallrecords_to_map_empty_vs_qdbus_cast COMMAND appimage-manager-tests dbus_getallrecords 1)
add_test(NAME dbus_getallrecords_two_records_each_round_trip COMMAND appimage-manager-tests dbus_getallrecords 2)
