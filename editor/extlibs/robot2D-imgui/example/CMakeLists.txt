cmake_minimum_required(VERSION 3.11)
project(robot2D-imgui-example)

set(CMAKE_CXX_STANDARD 17)
set(SRC main.cpp)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} robot2D_imgui)
