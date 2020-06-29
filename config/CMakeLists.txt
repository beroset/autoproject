cmake_minimum_required(VERSION 3.1)

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

INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/rules.txt DESTINATION share/autoproject/config)
INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/toplevel.cmake.txt DESTINATION share/autoproject/config)
INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/srclevel.cmake.txt DESTINATION share/autoproject/config)
INSTALL(FILES "${PROJECT_BINARY_DIR}/autoproject.conf" DESTINATION share/autoproject/config)
