cmake_minimum_required(VERSION 3.21)
project(robot2D VERSION 0.8.0)

include(cmake/Version.cmake)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_definitions(/MP)
endif()

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory)
	and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

cmake_policy(SET CMP0074 NEW)

if(RB2D_USE_CMAKE_VERBOSE)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif ()

include(cmake/Base.cmake)

enable_testing()

include(cmake/ShaderValidation.cmake)

add_subdirectory(core)
option(RB2D_BUILD_IMGUI "Build the optional Core ImGui integration" ON)
if(RB2D_BUILD_EDITOR)
    add_subdirectory(editor)
elseif(RB2D_BUILD_IMGUI)
    add_subdirectory(editor/extlibs/robot2D-imgui)
endif()

option(RB2D_CORE_GRAPHICS_TESTS "Register the Core and ImGui OpenGL 4.1 smoke" OFF)
if(RB2D_CORE_GRAPHICS_TESTS OR (RB2D_BUILD_CORE_TESTS AND TARGET robot2D-imgui))
    if(NOT TARGET robot2D-imgui)
        message(FATAL_ERROR "Core graphics smoke requires RB2D_BUILD_IMGUI")
    endif()
    add_executable(robot2D-core-imgui-smoke core/tests/Graphics/CoreImGuiSmoke.cpp)
    target_link_libraries(robot2D-core-imgui-smoke PRIVATE robot2D-core robot2D-imgui imgui)
    target_compile_definitions(robot2D-core-imgui-smoke PRIVATE
        RB2D_SMOKE_FONT="${CMAKE_CURRENT_SOURCE_DIR}/editor/res/fonts/notosans-regular.ttf")
    if(TARGET robot2D-core-tests)
        add_dependencies(robot2D-core-tests robot2D-core-imgui-smoke)
    endif()
    if(RB2D_CORE_GRAPHICS_TESTS)
        add_test(NAME core-imgui-opengl41 COMMAND robot2D-core-imgui-smoke)
        set_tests_properties(core-imgui-opengl41 PROPERTIES LABELS "graphics;core;imgui" TIMEOUT 60)
    endif()
endif()

include(cmake/CoreDocumentation.cmake)
