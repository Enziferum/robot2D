cmake_minimum_required(VERSION 3.21)
project(robot2D)

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

add_subdirectory(core)
add_subdirectory(editor)