set(INCLROOT ${PROJECT_SOURCE_DIR}/engine/include/robot2D)

set(ENGINE_SRC
        ${INCLROOT}/robot2D.hpp

        #Core
        ${INCLROOT}/Core/Event.hpp
        ${INCLROOT}/Core/Window.hpp
        ${INCLROOT}/Core/Keyboard.hpp
        ${INCLROOT}/Core/Vector2.hpp
        ${INCLROOT}/Core/MessageBus.hpp
        ${INCLROOT}/Core/Message.hpp
        ${INCLROOT}/Core/memory/Allocators.hpp
        ${INCLROOT}/Core/memory/MemoryManager.hpp

        #Ecs
        ${INCLROOT}/Ecs/Bitmask.hpp
        ${INCLROOT}/Ecs/Component.hpp
        ${INCLROOT}/Ecs/Defines.hpp
        ${INCLROOT}/Ecs/Entity.hpp
        ${INCLROOT}/Ecs/EntityManager.hpp
        ${INCLROOT}/Ecs/EventQueue.hpp
        ${INCLROOT}/Ecs/Scene.hpp
        ${INCLROOT}/Ecs/System.hpp
        ${INCLROOT}/Ecs/SystemManager.hpp

        #Graphics
        ${INCLROOT}/Graphics/Buffer.hpp
        ${INCLROOT}/Graphics/Color.hpp
        ${INCLROOT}/Graphics/Drawable.hpp
        ${INCLROOT}/Graphics/GL.hpp
        ${INCLROOT}/Graphics/Rect.hpp
        ${INCLROOT}/Graphics/RenderStates.hpp
        ${INCLROOT}/Graphics/RenderTarget.hpp
        ${INCLROOT}/Graphics/RenderWindow.hpp
        ${INCLROOT}/Graphics/Shader.hpp
        ${INCLROOT}/Graphics/Sprite.hpp
        ${INCLROOT}/Graphics/Texture.hpp
        ${INCLROOT}/Graphics/TextureAtlas.hpp
        ${INCLROOT}/Graphics/Transform.hpp
        ${INCLROOT}/Graphics/Transformable.hpp
        ${INCLROOT}/Graphics/View.hpp
        ${INCLROOT}/Graphics/VertexArray.hpp


        #Util
        ${INCLROOT}/Util/ResourceHandler.hpp
        ${INCLROOT}/Util/Logger.hpp
        PARENT_SCOPE
        )

set(SRCROOT ${PROJECT_SOURCE_DIR}/engine/src)

set(ENGINE_SRC ${ENGINE_SRC}
        #Core
        ${SRCROOT}/Core/Window.cpp
        ${SRCROOT}/Core/Event.cpp
        ${SRCROOT}/Core/WindowImpl.cpp
        ${SRCROOT}/Core/WindowImpl.hpp
        ${SRCROOT}/Core/WindowContext.cpp
        ${SRCROOT}/Core/Keyboard.cpp
        ${SRCROOT}/Core/Mouse.cpp
        ${SRCROOT}/Core/MessageBus.cpp
        ${SRCROOT}/Core/memory/Allocators.cpp
        ${SRCROOT}/Core/memory/MemoryManager.cpp

        #Graphics
        ${SRCROOT}/Graphics/Color.cpp
        ${SRCROOT}/Graphics/Drawable.cpp
        ${SRCROOT}/Graphics/Font.cpp
        ${SRCROOT}/Graphics/RenderStates.cpp
        ${SRCROOT}/Graphics/RenderWindow.cpp
        ${SRCROOT}/Graphics/RenderTarget.cpp
        ${SRCROOT}/Graphics/Shader.cpp
        ${SRCROOT}/Graphics/Sprite.cpp
        ${SRCROOT}/Graphics/Text.cpp
        ${SRCROOT}/Graphics/Texture.cpp
        ${SRCROOT}/Graphics/Transform.cpp
        ${SRCROOT}/Graphics/Transformable.cpp
        ${SRCROOT}/Graphics/RenderImpl.cpp
        ${SRCROOT}/Graphics/View.cpp
        ${SRCROOT}/Graphics/TextureAtlas.cpp
        ${SRCROOT}/Graphics/Buffer.cpp
        ${SRCROOT}/Graphics/VertexArray.cpp

        #Ecs
        ${SRCROOT}/Ecs/Bitmask.cpp
        ${SRCROOT}/Ecs/Component.cpp
        ${SRCROOT}/Ecs/Entity.cpp
        ${SRCROOT}/Ecs/EntityManager.cpp
        ${SRCROOT}/Ecs/EventQueue.cpp
        ${SRCROOT}/Ecs/Scene.cpp
        ${SRCROOT}/Ecs/System.cpp
        ${SRCROOT}/Ecs/SystemManager.cpp

        #Util
        ${SRCROOT}/Util/Logger.cpp

        # Impls
        ${SRCROOT}/Core/Desktop/DesktopWindowImpl.cpp
        ${SRCROOT}/Graphics/Desktop/OpenGLRender.cpp
        ${SRCROOT}/Graphics/Desktop/OpenGLBuffer.cpp
        ${SRCROOT}/Graphics/Desktop/OpenGLVertexArray.cpp

        PARENT_SCOPE
        )