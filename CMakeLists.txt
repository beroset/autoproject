cmake_minimum_required(VERSION 3.1)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(autoproject VERSION 2.1.0)
set(CMAKE_CXX_STANDARD 17)
set (CPACK_RESOURCE_FILE_LICENSE  
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VENDOR "beroset")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/*.swp;.git*")
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

# add the binary tree to the search path for include files
# so that we will find config.h
include_directories("${PROJECT_BINARY_DIR}")

# Create the documentation 
add_subdirectory(doc)

add_subdirectory(src)
add_subdirectory(bin)
add_subdirectory(config)

if (WITH_TEST)
    enable_testing()
    add_subdirectory(test)
endif() 

INCLUDE(InstallRequiredSystemLibraries)
include(CPack)
