cmake_minimum_required(VERSION 3.11)
project(robot2D-features)

set(CMAKE_CXX_STANDARD 14)

#todo cool and fancy cmake for feature module
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Render2D/incl/feature)
file(GLOB SRC Render2D/src/*.cpp)

set(LIBS robot2D)

add_library(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} ${LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Render2D/incl)
