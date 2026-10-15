# Opt-in target: ordinary engine builds do not require Python or glslang.
find_package(Python3 QUIET COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    add_custom_target(robot2D-check-shaders
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/validate_shaders.py"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Validating GLSL stages and linked programs"
        VERBATIM)
endif()
