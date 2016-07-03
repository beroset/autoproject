cmake_minimum_required(VERSION 2.8)
set(EXECUTABLE_NAME "autoproject")
add_executable(${EXECUTABLE_NAME} main.cpp AutoProject.cpp)
target_link_libraries(${EXECUTABLE_NAME} stdc++fs)
