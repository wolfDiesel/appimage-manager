qt_generate_moc(
  ${CMAKE_CURRENT_SOURCE_DIR}/directory_watcher.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/moc_directory_watcher.cpp
)
qt_generate_moc(
  ${CMAKE_CURRENT_SOURCE_DIR}/dbus_manager_adaptor.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/moc_dbus_manager_adaptor.cpp
)
add_executable(appimage-manager-daemon
  main.cpp
  directory_watcher.cpp
  dbus_manager_adaptor.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/moc_directory_watcher.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/moc_dbus_manager_adaptor.cpp
)
target_include_directories(appimage-manager-daemon PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(appimage-manager-daemon PRIVATE
  appimage-manager-core
  Qt6::Core
  Qt6::DBus
)

configure_file(appimage-manager.service.in ${CMAKE_CURRENT_BINARY_DIR}/appimage-manager.service @ONLY)

install(TARGETS appimage-manager-daemon RUNTIME DESTINATION bin)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/appimage-manager.service
  DESTINATION share/systemd/user
  COMPONENT systemd-user)
install(FILES config.json.example DESTINATION share/appimage-manager COMPONENT config)

if(BUILD_TESTS)
  add_executable(appimage-manager-daemon-adaptor-test
    test_dbus_adaptor.cpp
    dbus_manager_adaptor.cpp
    directory_watcher.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/moc_dbus_manager_adaptor.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/moc_directory_watcher.cpp
  )
  target_include_directories(appimage-manager-daemon-adaptor-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )
  target_link_libraries(appimage-manager-daemon-adaptor-test PRIVATE
    appimage-manager-core
    Qt6::Core
    Qt6::DBus
  )
  add_test(NAME daemon_adaptor_getallrecords_structure COMMAND appimage-manager-daemon-adaptor-test 0)
  add_test(NAME daemon_adaptor_getallrecords_empty COMMAND appimage-manager-daemon-adaptor-test 1)
  add_test(NAME daemon_adaptor_remove_appimage COMMAND appimage-manager-daemon-adaptor-test 2)
  add_test(NAME daemon_adaptor_remove_appimage_unknown_id COMMAND appimage-manager-daemon-adaptor-test 3)
endif()
