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

set(CMAKE_CXX_STANDARD 17)
# Some default variables which the user may change
option(CMAKE_BUILD_TYPE "Choose the type of build (Debug or Release)" Debug)
option(BUILD_SHARED_LIBS "Whether to build shared libraries" OFF)
option(BUILD_SANDBOX "Build Sandbox Submodule?" ON)
option(BUILD_EDITOR "Build Editor Submodule?" ON)


list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
cmake_policy(SET CMP0074 NEW)

set(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generated/${PROJECT_NAME}-config.cmake")
set(CONFIG_DEST "lib${LIB_SUFFIX}/cmake/${PROJECT_NAME}")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")

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
find_package(spdlog)
find_package(robot2D_ext QUIET)

if(WIN32)
    set(LIBS opengl32)
elseif(APPLE)
    include_directories(/System/Library/Frameworks)
    find_library(OpenGL_LIBRARY OpenGL)
    mark_as_advanced(OpenGL_LIBRARY)
    set(APPLE_LIBS ${OpenGL_LIBRARY} )
    set(APPLE_LIBS ${APPLE_LIBS})
    set(LIBS ${APPLE_LIBS})
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    set(LIBS OpenGL::GL)
endif()

# use spdlog for some time
add_subdirectory(engine)

add_library(${PROJECT_NAME} ${ENGINE_SRC})
set(LIBS ${LIBS} glfw robot2D_ext spdlog::spdlog)

target_link_libraries(${PROJECT_NAME} ${LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/include>"
        "$<INSTALL_INTERFACE:include>")

include(CMakePackageConfigHelpers)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/robot2D-config.cmake.in"
        ${CONFIG_FILE}
        INSTALL_DESTINATION ${CONFIG_DEST}
)

# Install
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/engine/include/robot2D DESTINATION include)

install(TARGETS ${PROJECT_NAME} EXPORT ${TARGETS_EXPORT_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(FILES ${CONFIG_FILE} DESTINATION ${CONFIG_DEST})

install( EXPORT ${TARGETS_EXPORT_NAME}
        DESTINATION "${CONFIG_DEST}")

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "ROBOT2D_STATIC")
endif()

if(BUILD_SANDBOX)
    add_subdirectory(sandbox)
endif()

if(BUILD_EDITOR)
    add_subdirectory(editor)
endif()
