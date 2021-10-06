/*********************************************************************
(c) Alex Raag 2021
https://github.com/Enziferum
robot2D - Zlib license.
This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/


#pragma once
#include <array>

#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Graphics/FrameBuffer.hpp>

#include <imgui/imgui.h>

namespace ImGui {

    class Wrapper {
    public:
        Wrapper();
        ~Wrapper();

        void init(robot2D::Window& window);
        void handleEvents(const robot2D::Event& event);
        void update(float dt);
        void render();
    private:

        void shutdown();
        void setupFonts();
        void updateMouseCursor();
        void setupRenderState(ImDrawData* draw_data, int fb_width, int fb_height,
                              unsigned int vertex_array_object);
        bool createDeviceObjects();

    private:
        robot2D::Window* m_window;

        bool m_windowHasFocus = true;
        bool m_mouseMoved = false;
        bool m_mousePressed[3] = {false, false, false};

        robot2D::Texture m_fontTexture;
        unsigned int VBO, EBO;

        robot2D::ShaderHandler m_shader;
        std::array<robot2D::Cursor, ImGuiMouseCursor_COUNT> m_cursors;
    };


    void Image(robot2D::Sprite& sprite, const robot2D::vec2f& size);
    void RenderFrameBuffer(const robot2D::FrameBuffer::Ptr& frameBuffer, const robot2D::vec2f& size);
}
