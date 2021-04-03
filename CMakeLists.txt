cmake_minimum_required(VERSION 3.7)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(autoproject VERSION 2.3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CPACK_RESOURCE_FILE_LICENSE  
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VENDOR "beroset")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/*.swp;.git*")
set(CPACK_PACKAGE_NAME autoproject)

# defines CMAKE_INSTALL_DATADIR and friends
include(GNUInstallDirs)

# options off-by-default that you can enable
option(WITH_TEST "Build the test suite" OFF)

# options on-by-default that you can disable
option(BUILD_DOCS "Build the documentation" ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    try_compile(HAS_FILESYSTEM "${CMAKE_BINARY_DIR}/temp" 
        "${CMAKE_SOURCE_DIR}/compilertests/has_filesystem.cpp" 
        CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
        LINK_LIBRARIES stdc++fs)
    if (NOT HAS_FILESYSTEM)
        try_compile(HAS_EXPERIMENTAL_FILESYSTEM "${CMAKE_BINARY_DIR}/temp" 
            "${CMAKE_SOURCE_DIR}/compilertests/has_experimental_filesystem.cpp" 
            CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
            LINK_LIBRARIES stdc++fs)
    endif()
else()
    try_compile(HAS_FILESYSTEM "${CMAKE_BINARY_DIR}/temp" 
        "${CMAKE_SOURCE_DIR}/compilertests/has_filesystem.cpp"
        CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON)
    if (NOT HAS_FILESYSTEM)
        try_compile(HAS_EXPERIMENTAL_FILESYSTEM "${CMAKE_BINARY_DIR}/temp" 
            "${CMAKE_SOURCE_DIR}/compilertests/has_experimental_filesystem.cpp"
            CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON)
    endif()
endif()

if(HAS_FILESYSTEM)
    message(STATUS "Compiler has filesystem support")
    set(HAS_FILESYSTEM 1)
else()
    if(HAS_EXPERIMENTAL_FILESYSTEM)
        message(STATUS "Compiler has experimental filesystem support")
        set(HAS_FILESYSTEM 0)
    else()
        message(FATAL_ERROR "Compiler is missing filesystem capabilities")
    endif(HAS_EXPERIMENTAL_FILESYSTEM)
endif(HAS_FILESYSTEM)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/src/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)

# Create the documentation
if (BUILD_DOCS)
    add_subdirectory(doc)
endif()

add_subdirectory(src)
add_subdirectory(bin)
add_subdirectory(config)
add_subdirectory(autodownload)

if (WITH_TEST)
    enable_testing()
    add_subdirectory(test)
endif() 

INCLUDE(InstallRequiredSystemLibraries)
include(CPack)
