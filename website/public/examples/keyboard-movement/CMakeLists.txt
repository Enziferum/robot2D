# Built inside a configured robot2D checkout, alongside lesson 01.
add_executable(robot2D-keyboard-movement main.cpp)
target_link_libraries(robot2D-keyboard-movement PRIVATE robot2D-core)
set_target_properties(robot2D-keyboard-movement PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
add_custom_command(TARGET robot2D-keyboard-movement POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../first-sprite/assets"
        "$<TARGET_FILE_DIR:robot2D-keyboard-movement>/assets"
    VERBATIM)

# Uses the published application and typed events, without opening a GL window.
add_executable(robot2D-keyboard-movement-test "${CMAKE_SOURCE_DIR}/website/tests/core-keyboard-movement.cpp")
target_link_libraries(robot2D-keyboard-movement-test PRIVATE robot2D-core)
foreach(scenario IN ITEMS held delta-time directions focus events)
    add_test(NAME core-lesson.keyboard-${scenario}
        COMMAND robot2D-keyboard-movement-test ${scenario})
    set_tests_properties(core-lesson.keyboard-${scenario} PROPERTIES
        LABELS "headless;core-lesson" TIMEOUT 30)
endforeach()
