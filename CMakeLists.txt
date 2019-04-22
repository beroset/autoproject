cmake_minimum_required(VERSION 3.1)
project(autoproject)
set(CMAKE_CXX_STANDARD 17)
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_SOURCE_IGNORE_FILES "/build/;/*.swp;.git*")
set(CPACK_PACKAGE_NAME autoproject)

# defines CMAKE_INSTALL_DATADIR and friends
include(GNUInstallDirs)

configure_file (
    "${PROJECT_SOURCE_DIR}/src/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)

# add the binary tree to the search path for include files
# so that we will find config.h
include_directories("${PROJECT_BINARY_DIR}")

add_subdirectory(src)
add_subdirectory(bin)
INCLUDE(InstallRequiredSystemLibraries)
#INSTALL(DIRECTORY dict DESTINATION share/words)
include(CPack)
