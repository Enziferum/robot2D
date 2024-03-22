include(${CMAKE_SOURCE_DIR}/cmake/Functions.cmake)
include(Options)

set(INCLROOT ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRCROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(IMGUI_INCL
        ${INCLROOT}/imgui/imconfig.h
        ${INCLROOT}/imgui/imgui.h
        ${INCLROOT}/imgui/imgui_internal.h
        ${INCLROOT}/imgui/imstb_rectpack.h
        ${INCLROOT}/imgui/imstb_textedit.h
        ${INCLROOT}/imgui/imstb_truetype.h
        ${INCLROOT}/imgui/ImGui.hpp)

set(IMGUI_SRC
        ${SRCROOT}/imgui/imgui.cpp
        ${SRCROOT}/imgui/imgui_widgets.cpp
        ${SRCROOT}/imgui/imgui_tables.cpp
        ${SRCROOT}/imgui/imgui_draw.cpp
        ${SRCROOT}/imgui/imgui_demo.cpp
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
        ${INCLROOT}/robot2D/imgui/Api.hpp
        ${INCLROOT}/robot2D/imgui/Gui.hpp
        ${INCLROOT}/robot2D/imgui/Util.hpp
        ${INCLROOT}/robot2D/imgui/WindowOptions.hpp
        ${INCLROOT}/robot2D/imgui/ScopedStyleColor.hpp
        ${INCLROOT}/robot2D/imgui/Spinner.hpp
        ${INCLROOT}/robot2D/imgui/Sugar.hpp
)

set(RB2D_IMGUI_SRC
        ${SRCROOT}/robot2D/Api.cpp
        ${SRCROOT}/robot2D/Gui.cpp
        ${SRCROOT}/robot2D/GuiImpl.hpp
        ${SRCROOT}/robot2D/GuiImpl.cpp
        ${SRCROOT}/robot2D/OrthoView.cpp
        ${SRCROOT}/robot2D/OrthoView.hpp
        ${SRCROOT}/robot2D/Render.cpp
        ${SRCROOT}/robot2D/Render.hpp
        ${SRCROOT}/robot2D/Util.cpp
        ${SRCROOT}/robot2D/WindowOptions.cpp
        ${SRCROOT}/robot2D/Spinner.cpp
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
