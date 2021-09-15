cmake_minimum_required(VERSION 3.11)
project(robot2D)

if(MSVC)
    add_definitions(/MP)
endif()

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory)
	and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

include(CheckCXXCompilerFlag)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
cmake_policy(SET CMP0074 NEW)

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
    #enable_cxx_compiler_flag_if_supported("-Werror")
endif()

find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)

message(STATUS "Found freetype in ${FREETYPE_INCLUDE_DIRS}")
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

find_package(robot2D_ext QUIET)

include_directories(${FREETYPE_INCLUDE_DIRS})

if(WIN32)
    set(LIBS opengl32)
else(APPLE)
    include_directories(/System/Library/Frameworks)
    find_library(OpenGL_LIBRARY OpenGL)
    mark_as_advanced(OpenGL_LIBRARY)
    set(APPLE_LIBS ${OpenGL_LIBRARY} )
    set(APPLE_LIBS ${APPLE_LIBS})
    set(LIBS ${APPLE_LIBS})
endif()

#Todo remove it
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/engine/include)
add_subdirectory(engine)


add_library(${PROJECT_NAME} ${ENGINE_SRC})
set(LIBS ${LIBS} glfw robot2D_ext ${FREETYPE_LIBRARIES})

target_link_libraries(${PROJECT_NAME} ${LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>")

add_subdirectory(sandbox)