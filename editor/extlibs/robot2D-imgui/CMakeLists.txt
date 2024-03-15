cmake_minimum_required(VERSION 3.21)
project(robot2D-imgui)

option(RB2D_IMGUI_BUILD_EXAMPLE "Build example application ?" ON)

if(MSVC)
    add_definitions(/MP)
endif()

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory)
	and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

set(CMAKE_CXX_STANDARD 17)

set (SRC
        include/imgui/imconfig.h
        include/imgui/imgui.h
        include/imgui/imgui_internal.h
        include/imgui/imstb_rectpack.h
        include/imgui/imstb_textedit.h
        include/imgui/imstb_truetype.h
        include/imgui/ImGui.hpp

        include/robot2D/imgui/Api.hpp
        include/robot2D/imgui/Gui.hpp
        include/robot2D/imgui/OrthoView.hpp
        include/robot2D/imgui/Util.hpp
        include/robot2D/imgui/Render.hpp
        include/robot2D/imgui/WindowOptions.hpp
        include/robot2D/imgui/Widgets.hpp
        include/robot2D/imgui/ScopedStyleColor.hpp
        include/robot2D/imgui/Spinner.hpp
        )

set(SRC ${SRC}
        src/imgui/imgui.cpp
        src/imgui/imgui_widgets.cpp
        src/imgui/imgui_tables.cpp
        src/imgui/imgui_draw.cpp
        src/imgui/imgui_demo.cpp

        src/robot2D/Api.cpp
        src/robot2D/Gui.cpp
        src/robot2D/OrthoView.cpp
        src/robot2D/Util.cpp
        src/robot2D/Render.cpp
        src/robot2D/WindowOptions.cpp
        src/robot2D/Widgets.cpp
        src/robot2D/Spinner.cpp
        )

add_library(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} robot2D-core)

if(RB2D_IMGUI_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()