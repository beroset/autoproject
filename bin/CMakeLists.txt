cmake_minimum_required(VERSION 3.0)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/fetchQ"
    "${CMAKE_BINARY_DIR}/fetchQ"
    COPYONLY
)
#install(TARGETS "${CMAKE_SOURCE_DIR}/bin/fetchQ" RESOURCE DESTINATION bin)
