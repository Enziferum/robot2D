cmake_minimum_required(VERSION 3.11)
project(robot2D-Sandbox)

set(CMAKE_CXX_STANDARD 17)

include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

if(MSVC)
    enable_cxx_compiler_flag_if_supported("/W3")
    # Show C4068 (unknown pragma) only on warning level 4
    enable_cxx_compiler_flag_if_supported("/w44068")
    # Show C4200 (zero-sized array in struct/union) only on warning level 4
    enable_cxx_compiler_flag_if_supported("/w44200")
else()
    enable_cxx_compiler_flag_if_supported("-Wall")
    enable_cxx_compiler_flag_if_supported("-Wextra")
    enable_cxx_compiler_flag_if_supported("-Wpedantic")
    enable_cxx_compiler_flag_if_supported("-Werror")
endif()

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
