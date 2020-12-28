find_package(Doxygen 1.8.16)
set(MD_SOURCES 01_intro.md )
set(IMAGE_PATH "images")
set(DEST_IMAGE_PATH "${CMAKE_BINARY_DIR}/${IMAGE_PATH}")
if(DOXYGEN_FOUND)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf.in"
        "${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf"
        @ONLY
    )
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/header.tex"
        "${CMAKE_CURRENT_BINARY_DIR}/header.tex"
        COPYONLY
    )
    add_custom_target(
        doc
        "${DOXYGEN_EXECUTABLE}"
        "${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf"
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/html/index.html"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        DEPENDS "${MD_SOURCES}"
        DEPENDS "${DEST_IMAGE_PATH}/npn.png"
        COMMENT "Generating code documentation..." VERBATIM
    )
    add_custom_target(
        pdf
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf"
    )
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf"
        COMMAND "make"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/index.html"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/latex"
        COMMENT "Generating PDF format code documentation..." VERBATIM
    )
    add_custom_command(
        OUTPUT "${DEST_IMAGE_PATH}/npn.png" 
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/${IMAGE_PATH}" "${DEST_IMAGE_PATH}"
        COMMENT "Copying image files..." VERBATIM
    )

endif(DOXYGEN_FOUND)
