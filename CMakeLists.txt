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

set(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generated/${PROJECT_NAME}-config.cmake")
set(CONFIG_DEST "lib${LIB_SUFFIX}/cmake/${PROJECT_NAME}")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(CMAKE_CXX_STANDARD 14)

#todo Build_Type option
#todo update config on build_type


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
    enable_cxx_compiler_flag_if_supported("/w44068") # Show C4068 (unknown pragma) only on warning level 4
    enable_cxx_compiler_flag_if_supported("/w44200") # Show C4200 (zero-sized array in struct/union) only on warning level 4
else()
    enable_cxx_compiler_flag_if_supported("-Wall")
    enable_cxx_compiler_flag_if_supported("-Wextra")
    enable_cxx_compiler_flag_if_supported("-Wpedantic")
    #enable_cxx_compiler_flag_if_supported("-Werror")
endif()


#install own-self
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)

message(STATUS "Found freetype in ${FREETYPE_INCLUDE_DIRS}")
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

find_package(robot2D_ext QUIET)

include_directories(${FREETYPE_INCLUDE_DIRS})

#todo cmake lists by module
set(SRC
        include/robot2D/robot2D.hpp

        #Core
        include/robot2D/Core/Event.hpp
        include/robot2D/Core/Window.hpp
        include/robot2D/Core/Keyboard.hpp
        include/robot2D/Core/Vector2.hpp

        #Extra
        include/robot2D/Extra/App.hpp
        include/robot2D/Extra/IStateMachine.hpp
        include/robot2D/Extra/State.hpp

        #Graphics
        include/robot2D/Graphics/Color.hpp
        include/robot2D/Graphics/Drawable.hpp
        include/robot2D/Graphics/GL.hpp
        include/robot2D/Graphics/Rect.hpp
        include/robot2D/Graphics/RenderStates.hpp
        include/robot2D/Graphics/RenderTarget.hpp
        include/robot2D/Graphics/RenderWindow.hpp
        include/robot2D/Graphics/Shader.hpp
        include/robot2D/Graphics/Sprite.hpp
        include/robot2D/Graphics/Texture.hpp
        include/robot2D/Graphics/Transform.hpp
        include/robot2D/Graphics/Transformable.hpp

        #Util
        include/robot2D/Util/ResourceHandler.hpp
        include/robot2D/Util/Logger.hpp
        src/Core/WindowImpl.hpp)

set(SRC
        ${SRC}
        #Core
        src/Core/Window.cpp
        src/Core/Event.cpp

        #Extra
        src/Extra/App.cpp
        src/Extra/State.cpp
        src/Extra/IStateMachine.cpp

        #Graphics
        src/Graphics/Color.cpp
        src/Graphics/Drawable.cpp
        src/Graphics/Font.cpp
        src/Graphics/RenderStates.cpp
        src/Graphics/RenderWindow.cpp
        src/Graphics/RenderTarget.cpp
        src/Graphics/Shader.cpp
        src/Graphics/Sprite.cpp
        src/Graphics/Text.cpp
        src/Graphics/Texture.cpp
        src/Graphics/Transform.cpp
        src/Graphics/Transformable.cpp
        src/Graphics/RenderImpl.cpp

        #Util
        src/Util/Logger.cpp
        src/Core/WindowImpl.cpp)

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

set(PLATFORM_SRC
        src/Core/Desktop/DesktopWindowImpl.cpp
        src/Graphics/Desktop/OpenGLRender.cpp)

#if()
#    target_compile_definitions(${target} PUBLIC "ROBOT2D_STATIC")
#endif()

add_library(${PROJECT_NAME} ${SRC} ${PLATFORM_SRC})
set(LIBS ${LIBS} glfw robot2D_ext ${FREETYPE_LIBRARIES})

target_link_libraries(${PROJECT_NAME} ${LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>")


include(CMakePackageConfigHelpers)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/robot2D-config.cmake.in"
        ${CONFIG_FILE}
        INSTALL_DESTINATION ${CONFIG_DEST}
)

# Install
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/robot2D DESTINATION include)

install(TARGETS ${PROJECT_NAME} EXPORT ${TARGETS_EXPORT_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(FILES ${CONFIG_FILE} DESTINATION ${CONFIG_DEST})

install( EXPORT ${TARGETS_EXPORT_NAME}
        DESTINATION "${CONFIG_DEST}")