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

# Compile and exercise the actual sample controller against the current public C# API.
set(platformer_project "${CMAKE_SOURCE_DIR}/examples/RobotLabPlatformer")
add_mono_assembly(TARGET robot2D-platformer-scripts
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../res/script"
    SOURCES "${platformer_project}/assets/scripts/PlayerController.cs"
    REFERENCES "${robot2D_ScriptCore_OUTPUT}"
    ADDITIONAL_ARGS -sdk:4.8 -langversion:7.2 -warnaserror+)
add_dependencies(robot2D-platformer-scripts robot2D_ScriptCore)
add_executable(robot2D-platformer-tests PlatformerTests.cpp)
target_link_libraries(robot2D-platformer-tests PRIVATE robot2D-editor-objects)
target_compile_definitions(robot2D-platformer-tests PRIVATE
    RB2D_TEST_SCRIPTCORE="${robot2D_ScriptCore_OUTPUT}"
    RB2D_TEST_PLATFORMER_ASSEMBLY="${robot2D-platformer-scripts_OUTPUT}"
    RB2D_TEST_PLATFORMER_PROJECT="${platformer_project}")
add_dependencies(robot2D-platformer-tests robot2D-platformer-scripts)
add_dependencies(robot2D-editor-test-suite robot2D-platformer-tests)
foreach(platformer_case IN ITEMS route recovery contacts boundaries)
    rb2d_register_test("editor-platformer-${platformer_case}" robot2D-platformer-tests "${platformer_case}" "headless;scripting;physics;game")
endforeach()

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
    foreach(scene_case IN ITEMS hierarchy runtime-camera-order editor-gui-scene hud-text asset-roundtrip missing-assets asset-identity animation-resources roundtrip invalid legacy prefab prefab-viewport save-dirty async-load close-project startup-failure)
        rb2d_register_test("editor-scene-${scene_case}" robot2D-scene-tests "${scene_case}" "graphics;integration")
        set_tests_properties("editor-scene-${scene_case}" PROPERTIES SKIP_RETURN_CODE 77)
    endforeach()
endif()

add_mono_assembly(TARGET robot2D-hud-driver
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../res/script"
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/HudDriver.cs" "${CMAKE_CURRENT_SOURCE_DIR}/../../docs/examples/HudExample.cs"
    REFERENCES "${robot2D_ScriptCore_OUTPUT}"
    ADDITIONAL_ARGS -sdk:4.8 -langversion:7.2 -warnaserror+)
add_dependencies(robot2D-hud-driver robot2D_ScriptCore)
add_executable(robot2D-hud-tests HudTests.cpp)
target_link_libraries(robot2D-hud-tests PRIVATE robot2D-editor-objects)
target_compile_definitions(robot2D-hud-tests PRIVATE
    RB2D_TEST_MONO_LIB="${INTERNAL_MONO_ASSEMBLY_DIR}"
    RB2D_TEST_MONO_ETC="${INTERNAL_MONO_CONFIG_DIR}"
    RB2D_TEST_SCRIPTCORE="${robot2D_ScriptCore_OUTPUT}"
    RB2D_TEST_HUD_DRIVER="${robot2D-hud-driver_OUTPUT}")
add_dependencies(robot2D-hud-tests robot2D-hud-driver)
add_dependencies(robot2D-editor-test-suite robot2D-hud-tests)
rb2d_register_test(editor-hud-managed robot2D-hud-tests managed "headless;scripting;hud")

# A managed consumer catches a missing/outdated API assembly in ordinary CI builds.
add_mono_assembly(TARGET robot2D-managed-api-tests TYPE exe
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../res/script"
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ManagedApiTests.cs"
    REFERENCES "${robot2D_ScriptCore_OUTPUT}"
    ADDITIONAL_ARGS -sdk:4.8 -langversion:7.2 -warnaserror+)
add_dependencies(robot2D-managed-api-tests robot2D_ScriptCore)
add_dependencies(robot2D-editor-test-suite robot2D-managed-api-tests)
add_test(NAME editor-managed-api COMMAND "${MONO_EXECUTABLE}" "${robot2D-managed-api-tests_OUTPUT}")
set_tests_properties(editor-managed-api PROPERTIES TIMEOUT 30 LABELS "editor;headless;scripting")

add_mono_assembly(TARGET robot2D-animation-driver
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../res/script"
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/AnimationDriver.cs"
    REFERENCES "${robot2D_ScriptCore_OUTPUT}"
    ADDITIONAL_ARGS -sdk:4.8 -langversion:7.2 -warnaserror+)
add_dependencies(robot2D-animation-driver robot2D_ScriptCore)
add_executable(robot2D-animation-tests AnimationTests.cpp)
target_link_libraries(robot2D-animation-tests PRIVATE robot2D-editor-objects)
target_compile_definitions(robot2D-animation-tests PRIVATE
    RB2D_TEST_MONO_LIB="${INTERNAL_MONO_ASSEMBLY_DIR}"
    RB2D_TEST_MONO_ETC="${INTERNAL_MONO_CONFIG_DIR}"
    RB2D_TEST_SCRIPTCORE="${robot2D_ScriptCore_OUTPUT}"
    RB2D_TEST_ANIMATION_DRIVER="${robot2D-animation-driver_OUTPUT}")
add_dependencies(robot2D-animation-tests robot2D-animation-driver)
add_dependencies(robot2D-editor-test-suite robot2D-animation-tests)
foreach(animation_case IN ITEMS frames playback clips parser managed runtime-clone)
    rb2d_register_test("editor-animation-${animation_case}" robot2D-animation-tests "${animation_case}" "headless;animation")
endforeach()

add_executable(robot2D-prefab-tests PrefabTests.cpp)
target_link_libraries(robot2D-prefab-tests PRIVATE robot2D-editor-objects)
add_dependencies(robot2D-editor-test-suite robot2D-prefab-tests)
foreach(prefab_case IN ITEMS instances invalid runtime package)
    rb2d_register_test("editor-prefab-${prefab_case}" robot2D-prefab-tests "${prefab_case}" "headless;prefab")
endforeach()

add_executable(robot2D-event-tests EventTests.cpp)
target_link_libraries(robot2D-event-tests PRIVATE robot2D-editor-objects robot2D-imgui)
add_dependencies(robot2D-editor-test-suite robot2D-event-tests)
foreach(event_case IN ITEMS variant order binder gui gui-capture capture shortcuts history)
    rb2d_register_test("editor-event-${event_case}" robot2D-event-tests "${event_case}" "headless;input")
endforeach()

add_executable(robot2D-scene-asset-tests SceneAssetTests.cpp)
target_link_libraries(robot2D-scene-asset-tests PRIVATE robot2D-editor-objects)
add_dependencies(robot2D-editor-test-suite robot2D-scene-asset-tests)
foreach(asset_case IN ITEMS compatibility invalid)
    rb2d_register_test("editor-scene-assets-${asset_case}" robot2D-scene-asset-tests "${asset_case}" "headless;assets")
endforeach()

add_mono_assembly(TARGET robot2D-physics-driver
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../res/script"
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/PhysicsDriver.cs"
    REFERENCES "${robot2D_ScriptCore_OUTPUT}"
    ADDITIONAL_ARGS -sdk:4.8 -langversion:7.2 -warnaserror+)
add_dependencies(robot2D-physics-driver robot2D_ScriptCore)
add_executable(robot2D-physics-tests PhysicsTests.cpp)
target_link_libraries(robot2D-physics-tests PRIVATE robot2D-editor-objects)
target_compile_definitions(robot2D-physics-tests PRIVATE
    RB2D_TEST_SCRIPTCORE="${robot2D_ScriptCore_OUTPUT}"
    RB2D_TEST_PHYSICS_DRIVER="${robot2D-physics-driver_OUTPUT}")
add_dependencies(robot2D-physics-tests robot2D-physics-driver)
add_dependencies(robot2D-editor-test-suite robot2D-physics-tests)
foreach(physics_case IN ITEMS geometry walls bullet triggers filters lifecycle callbacks invalid serialization managed)
    rb2d_register_test("editor-physics-${physics_case}" robot2D-physics-tests "${physics_case}" "headless;physics")
endforeach()

add_executable(robot2D-gui-backend-tests GuiBackendTests.cpp)
target_link_libraries(robot2D-gui-backend-tests PRIVATE robot2D-imgui imgui)
target_include_directories(robot2D-gui-backend-tests PRIVATE ../extlibs/robot2D-imgui/src/robot2D)
target_compile_definitions(robot2D-gui-backend-tests PRIVATE RB2D_TEST_FONT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../res/fonts")
add_dependencies(robot2D-editor-test-suite robot2D-gui-backend-tests)
foreach(gui_case IN ITEMS math uninitialized fonts)
    rb2d_register_test("imgui-backend-${gui_case}" robot2D-gui-backend-tests "${gui_case}" "headless;imgui")
endforeach()
if(RB2D_EDITOR_GRAPHICS_TESTS)
    foreach(gui_api IN ITEMS 43 45)
        foreach(gui_case IN ITEMS lifecycle renderer rollback)
            set(gui_name "imgui-backend-${gui_case}-${gui_api}")
            rb2d_register_test("${gui_name}" robot2D-gui-backend-tests "${gui_case}" "graphics;imgui" "${gui_api}")
            set_tests_properties("${gui_name}" PROPERTIES SKIP_RETURN_CODE 77)
        endforeach()
    endforeach()
endif()

add_executable(robot2D-script-project-tests ScriptProjectTests.cpp)
target_link_libraries(robot2D-script-project-tests PRIVATE robot2D-editor-objects)
target_compile_definitions(robot2D-script-project-tests PRIVATE
    RB2D_TEST_SCRIPT_PYTHON="${Python3_EXECUTABLE}"
    RB2D_TEST_SCRIPT_GENERATOR="${CMAKE_CURRENT_SOURCE_DIR}/../res/gen/gen_project.py"
    RB2D_TEST_SCRIPTCORE="${robot2D_ScriptCore_OUTPUT}"
    RB2D_TEST_SCRIPT_MCS="${MONO_MCS_EXECUTABLE}"
    RB2D_TEST_SCRIPT_MONO="${MONO_EXECUTABLE}")
add_dependencies(robot2D-script-project-tests robot2D_ScriptCore)
add_dependencies(robot2D-editor-test-suite robot2D-script-project-tests)
foreach(script_case IN ITEMS project errors reload)
    rb2d_register_test("editor-script-${script_case}" robot2D-script-project-tests "${script_case}" "headless;scripting")
endforeach()

add_executable(robot2D-desktop-window-tests DesktopWindowTests.cpp)
target_link_libraries(robot2D-desktop-window-tests PRIVATE robot2D-editor-objects)
add_dependencies(robot2D-editor-test-suite robot2D-desktop-window-tests)
rb2d_register_test(core-desktop-uninitialized robot2D-desktop-window-tests uninitialized "headless;core;desktop")
rb2d_register_test(core-desktop-editor-shutdown robot2D-desktop-window-tests editor-shutdown "headless;core;desktop")
if(RB2D_EDITOR_GRAPHICS_TESTS)
    foreach(desktop_case IN ITEMS lifetime render-lifetime rollback borrowed events engine)
        rb2d_register_test("core-desktop-${desktop_case}" robot2D-desktop-window-tests "${desktop_case}" "graphics;core;desktop")
        set_tests_properties("core-desktop-${desktop_case}" PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
    endforeach()
endif()
