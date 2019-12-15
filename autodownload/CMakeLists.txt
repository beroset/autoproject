cmake_minimum_required(VERSION 3.0)
configure_file(
    "com.beroset.autoproject.json"
    "com.beroset.autoproject.json"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/com.beroset.autoproject.json" 
    DESTINATION "/usr/lib64/mozilla/native-messaging-hosts/")
