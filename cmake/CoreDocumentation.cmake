option(RB2D_BUILD_DOCS "Generate Core API documentation" OFF)

if(RB2D_BUILD_DOCS)
    find_package(Doxygen REQUIRED)
    # Relative paths work with both native Windows and MSYS Doxygen executables.
    file(RELATIVE_PATH RB2D_DOC_SOURCE "${CMAKE_BINARY_DIR}" "${CMAKE_SOURCE_DIR}")
    configure_file("${CMAKE_SOURCE_DIR}/docs/Doxyfile.core.in"
                   "${CMAKE_BINARY_DIR}/Doxyfile.robot2D-core-docs" @ONLY)
    add_custom_target(robot2D-core-docs
        COMMAND Doxygen::doxygen Doxyfile.robot2D-core-docs
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Generate robot2D Core Util API documentation"
        VERBATIM)
endif()
