find_package(Doxygen 1.8.16)
set(MD_SOURCES 01_intro.md )
set(IMAGE_PATH "images")
set(DEST_IMAGE_PATH "${CMAKE_BINARY_DIR}/${IMAGE_PATH}")
if(DOXYGEN_FOUND)
    configure_file(
        "${autoproject_SOURCE_DIR}/doc/doxygen.conf.in"
        "${CMAKE_BINARY_DIR}/doxygen.conf"
        @ONLY
    )
    configure_file(
        "${autoproject_SOURCE_DIR}/doc/header.tex"
        "${CMAKE_BINARY_DIR}/doc/header.tex"
        COPYONLY
    )
    add_custom_target(
        doc
        "${DOXYGEN_EXECUTABLE}"
        "${CMAKE_BINARY_DIR}/doxygen.conf"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        DEPENDS "${MD_SOURCES}"
        DEPENDS "${DEST_IMAGE_PATH}/npn.png"
        COMMENT "Generating API Reference documentation..." VERBATIM
    )
    add_custom_target(
        pdf
        DEPENDS "${CMAKE_BINARY_DIR}/doc/latex/refman.pdf"
    )
    add_custom_command(
        OUTPUT  "${CMAKE_BINARY_DIR}/doc/latex/refman.pdf"
        COMMAND "make"
        DEPENDS "doc"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/doc/latex"
        COMMENT "Generating PDF format Reference documentation..." VERBATIM
    )
    add_custom_command(
        OUTPUT "${DEST_IMAGE_PATH}/npn.png" 
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/${IMAGE_PATH}" "${DEST_IMAGE_PATH}"
        COMMENT "Copying image files..." VERBATIM
    )

endif(DOXYGEN_FOUND)
