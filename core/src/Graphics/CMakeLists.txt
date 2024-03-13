set(INCLROOT ${PROJECT_SOURCE_DIR}/include/robot2D/Graphics)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/Graphics)

set(GRAPHICS_INCLUDE_FILES
    ${INCLROOT}/Buffer.hpp
    ${INCLROOT}/Color.hpp
    ${INCLROOT}/Drawable.hpp
    ${INCLROOT}/GL.hpp
    ${INCLROOT}/Rect.hpp
    ${INCLROOT}/RenderContext.hpp
    ${INCLROOT}/RenderStates.hpp
    ${INCLROOT}/RenderTarget.hpp
    ${INCLROOT}/RenderWindow.hpp
    ${INCLROOT}/Shader.hpp
    ${INCLROOT}/Sprite.hpp
    ${INCLROOT}/Texture.hpp
    ${INCLROOT}/ImageAtlas.hpp
    ${INCLROOT}/Transform.hpp
    ${INCLROOT}/Transformable.hpp
    ${INCLROOT}/View.hpp
    ${INCLROOT}/VertexArray.hpp
    ${INCLROOT}/FrameBuffer.hpp
    ${INCLROOT}/Image.hpp
    ${INCLROOT}/RenderAPI.hpp
    ${INCLROOT}/Matrix3D.hpp
    ${INCLROOT}/Math3D.hpp
    ${INCLROOT}/Font.hpp
    ${INCLROOT}/Text.hpp
    ${INCLROOT}/QuadBatchRender.hpp
    PARENT_SCOPE)

set(GRAPHICS_SOURCE_FILES
    ${SRCROOT}/Color.cpp
    ${SRCROOT}/Drawable.cpp
    ${SRCROOT}/RenderContext.cpp
    ${SRCROOT}/RenderStates.cpp
    ${SRCROOT}/RenderWindow.cpp
    ${SRCROOT}/RenderTarget.cpp
    ${SRCROOT}/Shader.cpp
    ${SRCROOT}/Sprite.cpp
    ${SRCROOT}/Texture.cpp
    ${SRCROOT}/Transform.cpp
    ${SRCROOT}/Transformable.cpp
    ${SRCROOT}/RenderImpl.cpp
    ${SRCROOT}/View.cpp
    ${SRCROOT}/ImageAtlas.cpp
    ${SRCROOT}/Buffer.cpp
    ${SRCROOT}/VertexArray.cpp
    ${SRCROOT}/FrameBuffer.cpp
    ${SRCROOT}/Image.cpp
    ${SRCROOT}/RenderAPI.cpp
    ${SRCROOT}/Matrix3D.cpp
    ${SRCROOT}/Math3D.cpp
    ${SRCROOT}/Font.cpp
    ${SRCROOT}/Text.cpp

    #impl
    ${SRCROOT}/OpenGL/OpenGLRender.cpp
    ${SRCROOT}/OpenGL/OpenGLBuffer.cpp
    ${SRCROOT}/OpenGL/OpenGLVertexArray.cpp
    ${SRCROOT}/OpenGL/OpenGLFrameBuffer.cpp
    ${SRCROOT}/OpenGL/GL.cpp

    ${SRCROOT}/internal/glad.c
    ${SRCROOT}/internal/stb_image_aug.c

    PARENT_SCOPE)

# let CMake know about our additional graphics libraries paths
if(RB2D_OS_WINDOWS)
    list(APPEND CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/extlibs/headers/freetype2")
elseif(RB2D_OS_MACOS)
    list(APPEND CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/extlibs/headers/freetype2")
    list(APPEND CMAKE_LIBRARY_PATH "${PROJECT_SOURCE_DIR}/extlibs/libs-macos/Frameworks")
endif ()

# Image.cpp must be compiled with the -fno-strict-aliasing
# when gcc is used; otherwise saving PNGs may crash in stb_image_write
if(RB2D_COMPILER_GCC)
    set_source_files_properties(${SRCROOT}/Image.cpp PROPERTIES COMPILE_FLAGS -fno-strict-aliasing)
endif()