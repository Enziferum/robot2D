cmake_minimum_required(VERSION 3.21)
project(robot2D-core)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CompilerOptions.cmake)
process_logging_compiler_flags()
include(cmake/Process.cmake)

if(NOT RB2D_BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "ROBOT2D_CORE_STATIC")
endif()

if(RB2D_BUILD_SANDBOX)
    add_subdirectory(sandbox)
endif()

if(RB2D_BUILD_TESTS)
    add_subdirectory(tests)
endif()