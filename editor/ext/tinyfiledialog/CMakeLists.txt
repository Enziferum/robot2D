cmake_minimum_required(VERSION 3.11)
project(tinyfiledialogs)

if(MSVC)
    add_definitions(/MP)
endif()

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory)
	and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

set(CMAKE_C_STANDARD 99)

set(SRC src/tinyfiledialogs.c)

add_library(${PROJECT_NAME} STATIC ${SRC})
target_include_directories(${PROJECT_NAME} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>")