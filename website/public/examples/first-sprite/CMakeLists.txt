# Included from the robot2D root; this is not a standalone SDK project.
add_executable(robot2D-first-sprite main.cpp)
target_link_libraries(robot2D-first-sprite PRIVATE robot2D-core)
set_target_properties(robot2D-first-sprite PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
add_custom_command(TARGET robot2D-first-sprite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:robot2D-first-sprite>/assets"
    VERBATIM)

option(RB2D_CORE_EXAMPLE_GRAPHICS_TESTS "Test the Core lesson with a hidden GL window" OFF)
if(RB2D_CORE_EXAMPLE_GRAPHICS_TESTS)
    add_executable(robot2D-first-sprite-test "${CMAKE_SOURCE_DIR}/website/tests/core-first-sprite.cpp")
    target_include_directories(robot2D-first-sprite-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(robot2D-first-sprite-test PRIVATE robot2D-core)
    foreach(scenario IN ITEMS render missing-texture)
        add_test(NAME core-lesson.${scenario} COMMAND robot2D-first-sprite-test
            ${scenario} "${CMAKE_CURRENT_SOURCE_DIR}/assets/robot.png")
        set_tests_properties(core-lesson.${scenario} PROPERTIES LABELS "graphics;core-lesson"
            SKIP_RETURN_CODE 77 TIMEOUT 30)
    endforeach()
endif()
