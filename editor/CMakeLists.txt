cmake_minimum_required(VERSION 3.11)
project(robot2D-editor)

option(RB2D_EDITOR_USE_GLM "Use GLM?" OFF)
option(RB2D_EDITOR_THREAD_SANITIZER "Using thread sanitizer ?" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
if(RB2D_EDITOR_THREAD_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=thread")
endif ()

# must be preinstall
add_subdirectory(ext/robot2D-imgui)
add_subdirectory(ext/yaml-cpp)
add_subdirectory(ext/tinyfiledialog)
add_subdirectory(ext/filewatch)
add_subdirectory(ext/TinyXML)

include(cmake/Box2DPatching.cmake)
include(cmake/MonoPatching.cmake)

if(RB2D_EDITOR_USE_GLM)
    add_subdirectory(ext/glm)
    set(LIBS glm)
endif()

set(LIBS robot2D robot2D_imgui yaml-cpp tinyfiledialogs FileWatch box2d tinyxml ${MONO_LIBRARIES} ${LIBS})
file(GLOB_RECURSE SRC src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC})

if(UNIX)
    set(GCC_COVERAGE_LINK_FLAGS "-ldl -Wl --start-group")
    add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC ${MONO_INCLUDE_PATH})
target_include_directories(${PROJECT_NAME} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/
        ${CMAKE_CURRENT_BINARY_DIR}/res/)
		
if (WIN32)
	# TODO(a.raag): Correct copy mono bins 
	#message(STATUS "Save mono bin_path: ${MONO_BINARY_PATH}")
	#add_custom_command(TARGET ${PROJECT_NAME}
    #POST_BUILD
    #COMMAND ${CMAKE_COMMAND} 
        #-DMONO_BINARY_PATH=${MONO_BINARY_PATH}
        #-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MonoCopyRuntime.cmake
    #COMMENT "Copy runtime mono libs to executable..."
    #WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	#VERBATIM
    #)
endif ()		


		

		
