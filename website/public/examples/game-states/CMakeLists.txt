add_executable(robot2D-game-states main.cpp)
target_link_libraries(robot2D-game-states PRIVATE robot2D-core)
set_target_properties(robot2D-game-states PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
add_custom_command(TARGET robot2D-game-states POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../first-sprite/assets" "$<TARGET_FILE_DIR:robot2D-game-states>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../collect-items/assets" "$<TARGET_FILE_DIR:robot2D-game-states>/assets" VERBATIM)

add_executable(robot2D-game-states-test "${CMAKE_SOURCE_DIR}/website/tests/core-game-states.cpp")
target_link_libraries(robot2D-game-states-test PRIVATE robot2D-core)
foreach(scenario IN ITEMS pause input focus victory restart exit)
    add_test(NAME core-lesson.states-${scenario} COMMAND robot2D-game-states-test ${scenario})
    set_tests_properties(core-lesson.states-${scenario} PROPERTIES LABELS "headless;core-lesson" TIMEOUT 30)
endforeach()

if(RB2D_CORE_EXAMPLE_GRAPHICS_TESTS)
    add_executable(robot2D-game-states-graphics-test "${CMAKE_SOURCE_DIR}/website/tests/core-game-states-graphics.cpp")
    target_link_libraries(robot2D-game-states-graphics-test PRIVATE robot2D-core)
    add_test(NAME core-lesson.states-render COMMAND robot2D-game-states-graphics-test
        "${CMAKE_CURRENT_SOURCE_DIR}/../first-sprite/assets/robot.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/../collect-items/assets/SourceSansPro-Regular.ttf")
    set_tests_properties(core-lesson.states-render PROPERTIES LABELS "graphics;core-lesson" SKIP_RETURN_CODE 77 TIMEOUT 30)
endif()
