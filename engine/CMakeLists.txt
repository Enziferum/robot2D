set(INCLROOT ${CMAKE_CURRENT_SOURCE_DIR}/include/robot2D)

set(ENGINE_SRC ${ENGINE_SRC}
        ${INCLROOT}/robot2D.hpp

        #Core
        ${INCLROOT}/Core/Event.hpp
        ${INCLROOT}/Core/Window.hpp
        ${INCLROOT}/Core/Keyboard.hpp
        ${INCLROOT}/Core/Vector2.hpp


        #Graphics
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
        ${INCLROOT}/Graphics/Transform.hpp
        ${INCLROOT}/Graphics/Transformable.hpp
        ${INCLROOT}/Graphics/View.hpp
        ${INCLROOT}/Graphics/TextureAtlas.hpp

        #Util
        ${INCLROOT}/Util/ResourceHandler.hpp
        ${INCLROOT}/Util/Logger.hpp
        PARENT_SCOPE
        )

set(SRCROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(ENGINE_SRC ${ENGINE_SRC}
        #Core
        ${SRCROOT}/Core/Window.cpp
        ${SRCROOT}/Core/Event.cpp
        ${SRCROOT}/Core/WindowImpl.cpp
        ${SRCROOT}/Core/WindowImpl.hpp
        ${SRCROOT}/Core/WindowContext.cpp
        ${SRCROOT}/Core/Keyboard.cpp
        ${SRCROOT}/Core/Mouse.cpp

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

        #Util
        ${SRCROOT}/Util/Logger.cpp

        # Impls
        ${SRCROOT}/Core/Desktop/DesktopWindowImpl.cpp
        ${SRCROOT}/Graphics/Desktop/OpenGLRender.cpp

        PARENT_SCOPE
        )