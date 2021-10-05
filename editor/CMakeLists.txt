cmake_minimum_required(VERSION 3.11)
project(robot2D-editor)
set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/incl)
file(GLOB SRC src/*.cpp)

add_subdirectory(ext/robot2D-imgui)
add_subdirectory(ext/yaml-cpp)

set(LIBS robot2D robot2D_imgui yaml-cpp)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} ${LIBS})