cmake_minimum_required(VERSION 3.1)
project(autoproject)
set(CMAKE_CXX_STANDARD 17)
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set (CPACK_RESOURCE_FILE_LICENSE  
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VENDOR "beroset")
SET(CPACK_SOURCE_IGNORE_FILES "/build/;/*.swp;.git*")
set(CPACK_PACKAGE_NAME autoproject)

# defines CMAKE_INSTALL_DATADIR and friends
include(GNUInstallDirs)

# options off-by-default that you can enable
option(WITH_TEST "Build the test suite" OFF)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/src/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)

configure_file (
    "${PROJECT_SOURCE_DIR}/config/autoproject.conf.in"
    "${PROJECT_BINARY_DIR}/autoproject.conf"
)

configure_file (
    "${PROJECT_SOURCE_DIR}/config/autoprojecttest.conf.in"
    "${PROJECT_BINARY_DIR}/autoprojecttest.conf"
)

# add the binary tree to the search path for include files
# so that we will find config.h
include_directories("${PROJECT_BINARY_DIR}")

add_subdirectory(src)
add_subdirectory(bin)

if (WITH_TEST)
    enable_testing()
    add_subdirectory(test)
endif() 

INCLUDE(InstallRequiredSystemLibraries)
INSTALL(FILES config/rules.txt DESTINATION share/autoproject/config)
INSTALL(FILES config/toplevel.cmake.txt DESTINATION share/autoproject/config)
INSTALL(FILES config/srclevel.cmake.txt DESTINATION share/autoproject/config)
INSTALL(FILES "${PROJECT_BINARY_DIR}/autoproject.conf" DESTINATION share/autoproject/config)
include(CPack)
