cmake_minimum_required(VERSION 3.11)
project(FileWatch)

set(CMAKE_CXX_COMPILER 11)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)