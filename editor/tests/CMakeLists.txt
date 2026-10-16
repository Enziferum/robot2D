add_executable(robot2D-task-tests TaskQueueTests.cpp)
target_link_libraries(robot2D-task-tests PRIVATE robot2D-editor-objects)

add_executable(robot2D-editor-tests EditorTests.cpp)
target_link_libraries(robot2D-editor-tests PRIVATE robot2D-editor-objects)

add_executable(robot2D-scene-tests SceneTests.cpp)
target_link_libraries(robot2D-scene-tests PRIVATE robot2D-editor-objects)
target_compile_definitions(robot2D-scene-tests PRIVATE RB2D_TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../res")

add_executable(robot2D-version-tests VersionTests.cpp)
target_link_libraries(robot2D-version-tests PRIVATE robot2D-editor-objects)
target_include_directories(robot2D-version-tests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../generated/$<CONFIG>")

add_executable(robot2D-graphics-diagnostics-tests GraphicsDiagnosticsTests.cpp)
target_link_libraries(robot2D-graphics-diagnostics-tests PRIVATE robot2D-core)

add_executable(robot2D-batch-render-tests BatchRenderTests.cpp)
target_link_libraries(robot2D-batch-render-tests PRIVATE robot2D-core)

add_custom_target(robot2D-editor-test-suite DEPENDS robot2D-task-tests robot2D-editor-tests
        robot2D-scene-tests robot2D-version-tests robot2D-graphics-diagnostics-tests robot2D-batch-render-tests)

function(rb2d_register_test name executable scenario labels)
    set(test_directory "${CMAKE_CURRENT_BINARY_DIR}/scenarios/${name}")
    file(MAKE_DIRECTORY "${test_directory}")
    add_test(NAME "${name}" COMMAND "${executable}" "${scenario}" ${ARGN})
    set_tests_properties("${name}" PROPERTIES TIMEOUT 30
        LABELS "editor;${labels}" WORKING_DIRECTORY "${test_directory}")
    if(WIN32)
        get_filename_component(compiler_directory "${CMAKE_CXX_COMPILER}" DIRECTORY)
        set(test_path "${compiler_directory};${MONO_BINARY_PATH};$ENV{PATH}")
        string(REPLACE ";" "\;" test_path "${test_path}")
        set_tests_properties("${name}" PROPERTIES ENVIRONMENT "PATH=${test_path}")
    endif()
endfunction()

foreach(task_case IN ITEMS empty completion shutdown clear callback dispatch)
    rb2d_register_test("task-queue-${task_case}" robot2D-task-tests "${task_case}" "headless;async")
endforeach()

rb2d_register_test(editor-version robot2D-version-tests version "headless;version")

foreach(editor_case IN ITEMS selection asset-paths asset-import asset-package console-stream console-limits yaml-save payload
        config-roundtrip config-invalid cache-roundtrip cache-recovery cache-future cache-migration)
    rb2d_register_test("editor-${editor_case}" robot2D-editor-tests "${editor_case}" "headless")
endforeach()

if(WIN32)
    rb2d_register_test(editor-cache-write-failure robot2D-editor-tests cache-write-failure "headless;windows")
endif()

if(RB2D_EDITOR_GRAPHICS_TESTS)
    foreach(batch_api IN ITEMS 43 45)
        foreach(batch_case IN ITEMS capacity textures layers flush geometry three-d malformed mixed mixed-overflow camera viewport-resize window-resize)
            set(batch_name "core-batch-${batch_case}-${batch_api}")
            rb2d_register_test("${batch_name}" robot2D-batch-render-tests "${batch_case}" "graphics;integration" "${batch_api}")
            set_tests_properties("${batch_name}" PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
        endforeach()
    endforeach()
    foreach(graphics_case IN ITEMS diagnostics shaders)
        rb2d_register_test("core-graphics-${graphics_case}" robot2D-graphics-diagnostics-tests "${graphics_case}" "graphics;integration")
        set_tests_properties("core-graphics-${graphics_case}" PROPERTIES SKIP_RETURN_CODE 77)
    endforeach()
    foreach(scene_case IN ITEMS hierarchy runtime-camera-order asset-roundtrip missing-assets asset-identity animation-resources roundtrip invalid legacy prefab save-dirty async-load close-project startup-failure)
        rb2d_register_test("editor-scene-${scene_case}" robot2D-scene-tests "${scene_case}" "graphics;integration")
        set_tests_properties("editor-scene-${scene_case}" PROPERTIES SKIP_RETURN_CODE 77)
    endforeach()
endif()
