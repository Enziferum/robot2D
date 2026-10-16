add_executable(robot2D-player src/player/main.cpp)
set_target_properties(robot2D-player PROPERTIES OUTPUT_NAME Game
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/player" BUILD_RPATH "$ORIGIN")
target_link_libraries(robot2D-player PRIVATE robot2D-runtime)
if(MINGW)
    target_link_options(robot2D-player PRIVATE -municode)
endif()
add_dependencies(robot2D-player robot2D_ScriptCore)
add_custom_command(TARGET robot2D-player POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        "-DPLAYER=$<TARGET_FILE:robot2D-player>"
        "-DCORE=${robot2D_ScriptCore_OUTPUT}"
        "-DMONO_LIB=${INTERNAL_MONO_ASSEMBLY_DIR}"
        "-DMONO_ETC=${INTERNAL_MONO_CONFIG_DIR}"
        "-DMONO_BIN=${MONO_BINARY_PATH}"
        "-DCOMPILER=${CMAKE_CXX_COMPILER}"
        "-DOBJDUMP=${CMAKE_OBJDUMP}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PlayerBundle.cmake"
    VERBATIM)
