add_executable(robot2D-collect-items main.cpp)
target_link_libraries(robot2D-collect-items PRIVATE robot2D-core)
set_target_properties(robot2D-collect-items PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
add_custom_command(TARGET robot2D-collect-items POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../first-sprite/assets" "$<TARGET_FILE_DIR:robot2D-collect-items>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:robot2D-collect-items>/assets" VERBATIM)

add_executable(robot2D-collect-items-test "${CMAKE_SOURCE_DIR}/website/tests/core-collect-items.cpp")
target_link_libraries(robot2D-collect-items-test PRIVATE robot2D-core)
foreach(scenario IN ITEMS once overlap complete restart route)
    add_test(NAME core-lesson.items-${scenario} COMMAND robot2D-collect-items-test ${scenario})
    set_tests_properties(core-lesson.items-${scenario} PROPERTIES LABELS "headless;core-lesson" TIMEOUT 30)
endforeach()

if(RB2D_CORE_EXAMPLE_GRAPHICS_TESTS)
    add_executable(robot2D-collect-items-graphics-test "${CMAKE_SOURCE_DIR}/website/tests/core-collect-items-graphics.cpp")
    target_link_libraries(robot2D-collect-items-graphics-test PRIVATE robot2D-core)
    foreach(scenario IN ITEMS render missing-font)
        add_test(NAME core-lesson.items-${scenario} COMMAND robot2D-collect-items-graphics-test ${scenario}
            "${CMAKE_CURRENT_SOURCE_DIR}/../first-sprite/assets/robot.png"
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/SourceSansPro-Regular.ttf")
        set_tests_properties(core-lesson.items-${scenario} PROPERTIES LABELS "graphics;core-lesson" SKIP_RETURN_CODE 77 TIMEOUT 30)
    endforeach()
endif()
