cmake_minimum_required(VERSION 3.0)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/com.beroset.autoproject.json"
    "/usr/lib64/mozilla/native-messaging-hosts/com.beroset.autoproject.json"
    COPYONLY
)
install(PROGRAMS "com.beroset.autoproject.json" DESTINATION bin)
