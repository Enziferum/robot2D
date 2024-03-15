/*********************************************************************
(c) Alex Raag 2024
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
#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Graphics/Texture.hpp>

#include <robot2D/Graphics/Buffer.hpp>
#include <robot2D/Graphics/VertexArray.hpp>

#include <imgui/imgui.h>
#include "OrthoView.hpp"

namespace robot2D {

    struct IRenderContext {
        virtual ~IRenderContext() = 0;
        virtual void enableFeatures() = 0;
        virtual void actualize() = 0;
        virtual void restore() = 0;
    };

    class GuiRender {
    public:
        GuiRender();
        GuiRender(const GuiRender& other) = delete;
        GuiRender& operator=(const GuiRender& other) = delete;
        GuiRender(GuiRender&& other) = delete;
        GuiRender& operator=(GuiRender&& other) = delete;
        ~GuiRender();

        void setup();
        void render(ImDrawData* drawData);
    private:
        bool setupGL();
        void setupFonts();
        void setupRenderState(ImDrawData* draw_data, const robot2D::vec2i& framebufferSize, unsigned int);
    private:
        robot2D::Texture m_fontTexture;
        robot2D::ShaderHandler m_shader;
        unsigned VAO, VBO, EBO;

        robot2D::VertexArray::Ptr m_vertexArray;
        robot2D::VertexBuffer::Ptr m_vertexBuffer;
        robot2D::VertexBuffer::Ptr m_indexBuffer;


        std::unique_ptr<IRenderContext> m_renderContext = nullptr;
        OrthoView orthoView;
    };
} // namespace robot2D
