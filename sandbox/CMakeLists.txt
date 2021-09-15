cmake_minimum_required(VERSION 3.11)
project(robot2D-Sandbox)

set(CMAKE_CXX_STANDARD 14)

set(LIBS robot2D robot2D_ext)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/incl)
set(SRC
        src/main.cpp
        src/Render2DScene.cpp
        src/Sandbox.cpp
        src/Scene.cpp
        )

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} ${LIBS})
