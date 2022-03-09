cmake_minimum_required(VERSION 3.11)
project(robot2D-Sandbox)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenGL REQUIRED)
set(LIBS OpenGL::GL robot2D robot2D_ext ${CMAKE_DL_LIBS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/incl)

set(SRC
        src/main.cpp
        src/Render2DScene.cpp
        src/Sandbox.cpp
        src/Scene.cpp
        src/Components.cpp
        src/Systems.cpp
        )


add_executable(${PROJECT_NAME} ${SRC})

SET(GCC_COVERAGE_LINK_FLAGS "-ldl -Wl --start-group")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

target_link_libraries(${PROJECT_NAME} ${LIBS})
