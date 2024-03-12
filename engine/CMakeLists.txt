set(INCLROOT ${PROJECT_SOURCE_DIR}/engine/include/robot2D)
set(SRCROOT ${PROJECT_SOURCE_DIR}/engine/src)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    option(RB_DEBUG "Enable debugging options" ON)
endif()

configure_file(${INCLROOT}/OptionConfig.hpp.in ${INCLROOT}/OptionConfig.hpp @ONLY)

set(ENGINE_INCLUDES
        ${INCLROOT}/robot2D.hpp

        ${INCLROOT}/Engine.hpp
        ${INCLROOT}/Application.hpp

        #Core
        ${INCLROOT}/Core/FileInputStream.hpp
        ${INCLROOT}/Core/Event.hpp
        ${INCLROOT}/Core/Window.hpp
        ${INCLROOT}/Core/Keyboard.hpp
        ${INCLROOT}/Core/Joystick.hpp
        ${INCLROOT}/Core/Vector2.hpp
        ${INCLROOT}/Core/MessageBus.hpp
        ${INCLROOT}/Core/Message.hpp
        ${INCLROOT}/Core/Time.hpp
        ${INCLROOT}/Core/Clock.hpp
        ${INCLROOT}/Core/Cursor.hpp
        ${INCLROOT}/Core/memory/IAllocator.hpp
        ${INCLROOT}/Core/memory/StackAllocator.hpp
        ${INCLROOT}/Core/memory/MemoryManager.hpp

        #Ecs
        ${INCLROOT}/Ecs/Bitmask.hpp
        ${INCLROOT}/Ecs/Component.hpp
        ${INCLROOT}/Ecs/Defines.hpp
        ${INCLROOT}/Ecs/Entity.hpp
        ${INCLROOT}/Ecs/EntityManager.hpp
        ${INCLROOT}/Ecs/Scene.hpp
        ${INCLROOT}/Ecs/System.hpp
        ${INCLROOT}/Ecs/SystemManager.hpp

        #Graphics
        ${INCLROOT}/Graphics/Buffer.hpp
        ${INCLROOT}/Graphics/Color.hpp
        ${INCLROOT}/Graphics/Drawable.hpp
        ${INCLROOT}/Graphics/GL.hpp
        ${INCLROOT}/Graphics/Rect.hpp
        ${INCLROOT}/Graphics/RenderContext.hpp
        ${INCLROOT}/Graphics/RenderStates.hpp
        ${INCLROOT}/Graphics/RenderTarget.hpp
        ${INCLROOT}/Graphics/RenderWindow.hpp
        ${INCLROOT}/Graphics/Shader.hpp
        ${INCLROOT}/Graphics/Sprite.hpp
        ${INCLROOT}/Graphics/Texture.hpp
        ${INCLROOT}/Graphics/ImageAtlas.hpp
        ${INCLROOT}/Graphics/Transform.hpp
        ${INCLROOT}/Graphics/Transformable.hpp
        ${INCLROOT}/Graphics/View.hpp
        ${INCLROOT}/Graphics/VertexArray.hpp
        ${INCLROOT}/Graphics/FrameBuffer.hpp
        ${INCLROOT}/Graphics/Image.hpp
        ${INCLROOT}/Graphics/RenderAPI.hpp
        ${INCLROOT}/Graphics/Matrix3D.hpp
        ${INCLROOT}/Graphics/Math3D.hpp
        ${INCLROOT}/Graphics/Font.hpp
        ${INCLROOT}/Graphics/Text.hpp
        ${INCLROOT}/Graphics/QuadBatchRender.hpp


        #Util
        ${INCLROOT}/Util/ResourceHandler.hpp
        ${INCLROOT}/Util/Logger.hpp
        PARENT_SCOPE
        )

set(ENGINE_SRC
        ${SRCROOT}/Engine.cpp
        ${SRCROOT}/Application.cpp

		#Core
        ${SRCROOT}/Core/FileInputStream.cpp
        ${SRCROOT}/Core/Window.cpp
        ${SRCROOT}/Core/Event.cpp
        ${SRCROOT}/Core/WindowImpl.cpp
        ${SRCROOT}/Core/WindowImpl.hpp
        ${SRCROOT}/Core/WindowContext.cpp
        ${SRCROOT}/Core/Keyboard.cpp
        ${SRCROOT}/Core/Mouse.cpp
        ${SRCROOT}/Core/Joystick.cpp
        ${SRCROOT}/Core/Clock.cpp
        ${SRCROOT}/Core/Time.cpp
        ${SRCROOT}/Core/Cursor.cpp
        ${SRCROOT}/Core/CursorImpl.cpp
        ${SRCROOT}/Core/memory/IAllocator.cpp
        ${SRCROOT}/Core/memory/StackAllocator.cpp
        ${SRCROOT}/Core/memory/MemoryManager.cpp

        #Graphics
        ${SRCROOT}/Graphics/Color.cpp
        ${SRCROOT}/Graphics/Drawable.cpp
        ${SRCROOT}/Graphics/RenderContext.cpp
        ${SRCROOT}/Graphics/RenderStates.cpp
        ${SRCROOT}/Graphics/RenderWindow.cpp
        ${SRCROOT}/Graphics/RenderTarget.cpp
        ${SRCROOT}/Graphics/Shader.cpp
        ${SRCROOT}/Graphics/Sprite.cpp
        ${SRCROOT}/Graphics/Texture.cpp
        ${SRCROOT}/Graphics/Transform.cpp
        ${SRCROOT}/Graphics/Transformable.cpp
        ${SRCROOT}/Graphics/RenderImpl.cpp
        ${SRCROOT}/Graphics/View.cpp
        ${SRCROOT}/Graphics/ImageAtlas.cpp
        ${SRCROOT}/Graphics/Buffer.cpp
        ${SRCROOT}/Graphics/VertexArray.cpp
        ${SRCROOT}/Graphics/FrameBuffer.cpp
        ${SRCROOT}/Graphics/Image.cpp
        ${SRCROOT}/Graphics/RenderAPI.cpp
        ${SRCROOT}/Graphics/Matrix3D.cpp
        ${SRCROOT}/Graphics/Math3D.cpp
        ${SRCROOT}/Graphics/Font.cpp
        ${SRCROOT}/Graphics/Text.cpp

        #Ecs
        ${SRCROOT}/Ecs/Bitmask.cpp
        ${SRCROOT}/Ecs/Component.cpp
        ${SRCROOT}/Ecs/Entity.cpp
        ${SRCROOT}/Ecs/EntityManager.cpp
        ${SRCROOT}/Ecs/Scene.cpp
        ${SRCROOT}/Ecs/System.cpp
        ${SRCROOT}/Ecs/SystemManager.cpp

        #Util
        ${SRCROOT}/Util/Logger.cpp

        # Impls
        ${SRCROOT}/Core/Desktop/DesktopWindowImpl.cpp
        ${SRCROOT}/Core/Desktop/DesktopCursorImpl.cpp
        ${SRCROOT}/Graphics/OpenGL/OpenGLRender.cpp
        ${SRCROOT}/Graphics/OpenGL/OpenGLBuffer.cpp
        ${SRCROOT}/Graphics/OpenGL/OpenGLVertexArray.cpp
        ${SRCROOT}/Graphics/OpenGL/OpenGLFrameBuffer.cpp
        ${SRCROOT}/Graphics/OpenGL/GL.cpp

        ${SRCROOT}/Graphics/internal/glad.c
        ${SRCROOT}/Graphics/internal/stb_image_aug.c

        PARENT_SCOPE
        )