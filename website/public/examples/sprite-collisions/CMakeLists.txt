add_executable(robot2D-sprite-collisions main.cpp)
target_link_libraries(robot2D-sprite-collisions PRIVATE robot2D-core)
set_target_properties(robot2D-sprite-collisions PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
add_custom_command(TARGET robot2D-sprite-collisions POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../first-sprite/assets"
        "$<TARGET_FILE_DIR:robot2D-sprite-collisions>/assets" VERBATIM)

add_executable(robot2D-sprite-collisions-test "${CMAKE_SOURCE_DIR}/website/tests/core-sprite-collisions.cpp")
target_link_libraries(robot2D-sprite-collisions-test PRIVATE robot2D-core)
foreach(scenario IN ITEMS bounds directions sliding sweep application)
    add_test(NAME core-lesson.collision-${scenario} COMMAND robot2D-sprite-collisions-test ${scenario})
    set_tests_properties(core-lesson.collision-${scenario} PROPERTIES LABELS "headless;core-lesson" TIMEOUT 30)
endforeach()

if(RB2D_CORE_EXAMPLE_GRAPHICS_TESTS)
    add_executable(robot2D-sprite-collisions-graphics-test "${CMAKE_SOURCE_DIR}/website/tests/core-sprite-collisions-graphics.cpp")
    target_link_libraries(robot2D-sprite-collisions-graphics-test PRIVATE robot2D-core)
    add_test(NAME core-lesson.collision-render COMMAND robot2D-sprite-collisions-graphics-test
        "${CMAKE_CURRENT_SOURCE_DIR}/../first-sprite/assets/robot.png")
    set_tests_properties(core-lesson.collision-render PROPERTIES LABELS "graphics;core-lesson" SKIP_RETURN_CODE 77 TIMEOUT 30)
endif()
