include(${CMAKE_SOURCE_DIR}/cmake/Functions.cmake)
include(Options)

set(IMGUI_INCL
        include/imgui/imconfig.h
        include/imgui/imgui.h
        include/imgui/imgui_internal.h
        include/imgui/imstb_rectpack.h
        include/imgui/imstb_textedit.h
        include/imgui/imstb_truetype.h
        include/imgui/ImGui.hpp)

set(IMGUI_SRC
        src/imgui/imgui.cpp
        src/imgui/imgui_widgets.cpp
        src/imgui/imgui_tables.cpp
        src/imgui/imgui_draw.cpp
        src/imgui/imgui_demo.cpp
)

add_library(imgui STATIC ${IMGUI_INCL} ${IMGUI_SRC})
target_include_directories(imgui
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(imgui PRIVATE robot2D-core)

if (RB2D_OS_WINDOWS)
    if(RB2D_COMPILER_MSVC)
        GroupSourcesByFolder(imgui)
    endif ()
endif ()

set(RB2D_IMGUI_INCL
        include/robot2D/imgui/Api.hpp
        include/robot2D/imgui/Gui.hpp
        include/robot2D/imgui/Util.hpp
        include/robot2D/imgui/WindowOptions.hpp
        include/robot2D/imgui/ScopedStyleColor.hpp
        include/robot2D/imgui/Spinner.hpp
        include/robot2D/imgui/Sugar.hpp
)

set(RB2D_IMGUI_SRC
        src/robot2D/Api.cpp
        src/robot2D/Gui.cpp
        src/robot2D/GuiImpl.hpp
        src/robot2D/GuiImpl.cpp
        src/robot2D/OrthoView.cpp
        src/robot2D/OrthoView.hpp
        src/robot2D/Render.cpp
        src/robot2D/Render.hpp
        src/robot2D/Util.cpp
        src/robot2D/WindowOptions.cpp
        src/robot2D/Spinner.cpp
)


add_library(${PROJECT_NAME} ${RB2D_IMGUI_INCL} ${RB2D_IMGUI_SRC})

if (RB2D_OS_WINDOWS)
    if(RB2D_COMPILER_MSVC)
        GroupSourcesByFolder(${PROJECT_NAME})
    endif ()
endif ()


target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
target_link_libraries(${PROJECT_NAME} PUBLIC robot2D-core)

if(RB2D_IMGUI_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
