cmake_minimum_required(VERSION 3.20)

configure_file (
    "${CMAKE_CURRENT_LIST_DIR}/autoproject.conf.in"
    "${PROJECT_BINARY_DIR}/autoproject.conf"
)

if (WITH_TEST)
configure_file (
    "${CMAKE_CURRENT_LIST_DIR}/autoprojecttest.conf.in"
    "${PROJECT_BINARY_DIR}/autoprojecttest.conf"
)
endif()

INSTALL(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/c DESTINATION share/autoproject/config)
INSTALL(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/cpp DESTINATION share/autoproject/config)
INSTALL(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/asm DESTINATION share/autoproject/config)
INSTALL(FILES "${PROJECT_BINARY_DIR}/autoproject.conf" DESTINATION share/autoproject/config)
