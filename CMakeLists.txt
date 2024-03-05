cmake_minimum_required(VERSION 3.21)
project(robot2D)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_definitions(/MP)
endif()

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory)
	and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

if(RB2D_USE_CMAKE_VERBOSE)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
cmake_policy(SET CMP0074 NEW)

include(cmake/ProcessCore.cmake)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "ROBOT2D_STATIC")
endif()

if(RB2D_BUILD_SANDBOX)
    add_subdirectory(sandbox)
endif()

if(RB2D_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(RB2D_BUILD_EDITOR)
    add_subdirectory(editor)
endif()
