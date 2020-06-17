cmake_minimum_required(VERSION 3.1)
include_directories( ${CMAKE_HOME_DIRECTORY}/src )
add_executable(ConfigFileTest ConfigFileTest.cpp)
add_test(ConfigFileTest ConfigFileTest)

target_link_libraries(ConfigFileTest ConfigFile cppunit)
