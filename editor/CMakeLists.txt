include(${CMAKE_SOURCE_DIR}/cmake/Functions.cmake)

option(RB2D_EDITOR_USE_GLM "Use GLM?" OFF)
option(RB2D_EDITOR_THREAD_SANITIZER "Using thread sanitizer ?" OFF)
option(RB2D_BUILD_EDITOR_TESTS "Build editor regression tests" OFF)
option(RB2D_EDITOR_GRAPHICS_TESTS "Register editor tests requiring an OpenGL context" OFF)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
if(RB2D_EDITOR_THREAD_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=thread")
endif ()

# must be preinstall
add_subdirectory(extlibs/robot2D-imgui)
include(cmake/YamlCppPatching.cmake)
add_subdirectory(extlibs/tinyfiledialog)
add_subdirectory(extlibs/filewatch)
add_subdirectory(extlibs/TinyXML)
add_subdirectory(extlibs/RBIni)

include(cmake/Box2DPatching.cmake)
include(cmake/MonoPatching.cmake)

if(RB2D_EDITOR_USE_GLM)
    add_subdirectory(ext/glm)
    set(LIBS glm)
endif()

set(LIBS robot2D-core robot2D-imgui yaml-cpp tinyfiledialogs FileWatch box2d tinyxml rbini ${MONO_LIBRARIES} ${LIBS})

file(GLOB_RECURSE INCLS include/editor/*.hpp)
file(GLOB_RECURSE SRC src/*.cpp)

list(REMOVE_ITEM SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
add_library(robot2D-editor-objects OBJECT ${INCLS} ${SRC})
add_executable(${PROJECT_NAME} src/main.cpp)

include(cmake/ScriptCore.cmake)
find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
configure_file(cmake/ScriptBuildConfig.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/generated/editor/ScriptBuildConfig.hpp" @ONLY)
target_include_directories(robot2D-editor-objects PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")

include(cmake/BuildInfo.cmake)
target_include_directories(robot2D-editor-objects PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated/$<CONFIG>")

if (RB2D_OS_WINDOWS)
    if(RB2D_COMPILER_MSVC)
        GroupSourcesByFolder(${PROJECT_NAME})
    endif ()
endif ()


if(UNIX)
    set(GCC_COVERAGE_LINK_FLAGS "-ldl -Wl --start-group")
    add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})
endif()

target_include_directories(robot2D-editor-objects PUBLIC ${MONO_INCLUDE_PATH}
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

target_link_libraries(robot2D-editor-objects PUBLIC ${LIBS})
target_link_libraries(${PROJECT_NAME} PRIVATE robot2D-editor-objects)

if(RB2D_BUILD_EDITOR_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/
        ${CMAKE_CURRENT_BINARY_DIR}/res/)

if (RB2D_OS_WINDOWS)
	add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
            -DDESTINATION_PATH=${CMAKE_CURRENT_BINARY_DIR}
            -DMONO_BINARY_PATH=${MONO_BINARY_PATH}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MonoCopyRuntime.cmake
    COMMENT "Copy runtime mono libs to executable..."
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	VERBATIM
    )
endif ()


		

		
