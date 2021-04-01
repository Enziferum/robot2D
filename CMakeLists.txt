cmake_minimum_required(VERSION 3.11)
project(robot2D)

if(MSVC)
    add_definitions(/MP)
endif()

option(BUILD_GLAD "Build glad library" ON)
option(BUILD_STB_IMAGE "Build stb_image library" ON)
option(BUILD_EDITOR "" ON)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory)
	and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
cmake_policy(SET CMP0074 NEW)

set(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generated/${PROJECT_NAME}-config.cmake")
set(CONFIG_DEST "lib${LIB_SUFFIX}/cmake/${PROJECT_NAME}")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")

#todo set compiler flag options

set(CMAKE_CXX_STANDARD 14)

#install own-self
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)

message(STATUS "Found freetype in ${FREETYPE_INCLUDE_DIRS}")
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

find_package(robot2D_ext QUIET)

include_directories(${FREETYPE_INCLUDE_DIRS})

set(SRC
        #Core
        include/robot2D/Core/App.h
        include/robot2D/Core/Event.h
        include/robot2D/Core/IStateMachine.h
        include/robot2D/Core/Window.h
        include/robot2D/Core/Keyboard.h
        include/robot2D/Core/Vector2.h

        #Graphics
        include/robot2D/Graphics/Color.h
        include/robot2D/Graphics/Drawable.h
        include/robot2D/Graphics/GL.h
        include/robot2D/Graphics/Rect.h
        include/robot2D/Graphics/RenderStates.h
        include/robot2D/Graphics/RenderTarget.h
        include/robot2D/Graphics/RenderWindow.h
        include/robot2D/Graphics/Shader.h
        include/robot2D/Graphics/Sprite.h
        include/robot2D/Graphics/Texture.h
        include/robot2D/Graphics/Transform.h
        include/robot2D/Graphics/Transformable.h

        #Util
        include/robot2D/Util/ResourceHandler.h
        )

set(SRC
        ${SRC}
        #Core

        src/Core/App.cpp
        src/Core/IStateMachine.cpp
        src/Core/Window.cpp
        src/Core/Event.cpp
        src/Core/State.cpp

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

        #Util
        )

if(WIN32)
    set(LIBS opengl32)
endif()

add_library(${PROJECT_NAME} ${SRC})
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