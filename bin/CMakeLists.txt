cmake_minimum_required(VERSION 3.20)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/fetchQ"
    "${CMAKE_BINARY_DIR}/fetchQ"
    COPYONLY
)
install(PROGRAMS "fetchQ" DESTINATION bin)
