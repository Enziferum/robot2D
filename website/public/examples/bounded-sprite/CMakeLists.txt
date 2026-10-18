add_executable(robot2D-bounded-sprite main.cpp)
target_link_libraries(robot2D-bounded-sprite PRIVATE robot2D-core)
set_target_properties(robot2D-bounded-sprite PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
add_custom_command(TARGET robot2D-bounded-sprite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../first-sprite/assets"
        "$<TARGET_FILE_DIR:robot2D-bounded-sprite>/assets" VERBATIM)

add_executable(robot2D-core-lessons-test "${CMAKE_SOURCE_DIR}/website/tests/core-lessons-03-05.cpp")
target_link_libraries(robot2D-core-lessons-test PRIVATE robot2D-core)
foreach(scenario IN ITEMS origins edges resize animation camera)
    add_test(NAME core-lesson.${scenario} COMMAND robot2D-core-lessons-test ${scenario})
    set_tests_properties(core-lesson.${scenario} PROPERTIES LABELS "headless;core-lesson" TIMEOUT 30)
endforeach()
