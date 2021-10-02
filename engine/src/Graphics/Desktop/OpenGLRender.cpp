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

#include <stdexcept>
#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Buffer.hpp>

#include "OpenGLRender.hpp"

namespace robot2D {
    namespace priv {

        const std::string vertexSource = R"(
            #version 330 core
            layout (location = 0) in vec2 position;
            layout (location = 1) in vec2 textureCoords;
            layout (location = 2) in vec4 color;
            layout (location = 3) in float textureIndex;
            out vec2 TexCoords;
            out vec4 Color;
            out float TexIndex;
            uniform mat4 projection;
            void main()
            {
                TexCoords = textureCoords;
                Color = color;
                TexIndex = textureIndex;
                gl_Position = projection * vec4(position, 0.0, 1.0);
            }
        )";

        const std::string fragmentSource = R"(
            #version 330 core
            in vec2 TexCoords;
            in vec4 Color;
            in float TexIndex;
            uniform sampler2D sprite;
            out vec4 fragColor;
            void main()
            {
                fragColor = Color * texture(sprite, TexCoords);
            }
        )";

        constexpr short quadVertexSize = 4;


        OpenGLRender::OpenGLRender() {}

        OpenGLRender::~OpenGLRender() {
            destroy();
        }

        // TODO create Buffers(Vertex, Index)
        void OpenGLRender::init() {
            m_renderBuffer.quadBuffer = new RenderVertex[m_renderBuffer.maxQuadsCount];

            auto vertexBuffer = VertexBuffer::Create(sizeof(RenderVertex) * m_renderBuffer.maxQuadsCount);



            glGenVertexArrays(1, &m_renderBuffer.VAO);
            glGenBuffers(1, &m_renderBuffer.VBO);
            glGenBuffers(1, &m_renderBuffer.EBO);

            glBindBuffer(GL_ARRAY_BUFFER, m_renderBuffer.VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(RenderVertex) * m_renderBuffer.maxQuadsCount, nullptr, GL_DYNAMIC_DRAW);

            glBindVertexArray(m_renderBuffer.VAO);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), nullptr);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (const void*) offsetof(RenderVertex, TextureCoords));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (const void*) offsetof(RenderVertex, Color));
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (const void*) offsetof(RenderVertex, textureIndex));

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderBuffer.EBO);
            uint32_t* quadIndices = new uint32_t[m_renderBuffer.maxIndicesCount];



            uint32_t offset = 0;
            for (uint32_t i = 0; i < m_renderBuffer.maxIndicesCount; i += 6)
            {
                quadIndices[i + 0] = offset + 0;
                quadIndices[i + 1] = offset + 1;
                quadIndices[i + 2] = offset + 2;

                quadIndices[i + 3] = offset + 2;
                quadIndices[i + 4] = offset + 3;
                quadIndices[i + 5] = offset + 0;

                offset += 4;
            }
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_renderBuffer.maxIndicesCount, quadIndices, GL_STATIC_DRAW);
            auto indexBuffer = IndexBuffer::Create(quadIndices, m_renderBuffer.maxIndicesCount);
            delete[] quadIndices;


            m_renderBuffer.quadVertexPositions[0] = {-0.5F, -0.5F};
            m_renderBuffer.quadVertexPositions[1] = {0.5F, -0.5F};
            m_renderBuffer.quadVertexPositions[2] = {0.5F, 0.5F};
            m_renderBuffer.quadVertexPositions[3] = {-0.5F, 0.5F};

            if(!m_quadShader.createShader(shaderType::vertex,
                                             vertexSource, false)) {
                std::string reason = "Can't load Quad Vertex Shader";
                throw std::runtime_error(reason);
            }

            if(!m_quadShader.createShader(shaderType::fragment,
                                             fragmentSource, false)) {
                std::string reason = "Can't load Quad Fragment Shader";
                throw std::runtime_error(reason);
            }

            uint32_t textureData = 0xffffffff;
            m_renderBuffer.whiteTexture.create({1, 1}, &textureData);
            m_renderBuffer.textureSlots[0] = m_renderBuffer.whiteTexture.getID();

            m_quadShader.use();
//            for(int it = 0; it < 32; ++it)
//                m_quadShader.set_parameter("textureSamplers", it);
            m_quadShader.set_parameter("sprite", 0);
            for(int it = 1; it < 2; ++it)
                m_renderBuffer.textureSlots[it] = 0;

            m_default.reset(FloatRect(0.F, 0.F,
                                      static_cast<float>(m_size.x),
                                      static_cast<float>(m_size.y)));
            m_view = m_default;
            m_quadShader.set_parameter("projection", m_view.getTransform().get_matrix());
        }


        void OpenGLRender::destroy() {
            glDeleteVertexArrays(1, &m_renderBuffer.VAO);
            glDeleteBuffers(1, &m_renderBuffer.VBO);
            glDeleteBuffers(1, &m_renderBuffer.EBO);

            delete[] m_renderBuffer.quadBuffer;
        }


        void OpenGLRender::setView(const View& view) {
            m_view = view;
            applyCurrentView();
        }


        void OpenGLRender::render(const VertexData& data, const RenderStates& states) const {
            // Rendering not quads in version 0.2 - 0.3 not supported todo runtime error
            if(data.size() != quadVertexSize)
                return;
            // Default Quad Drawing

            if(m_renderBuffer.indexCount >= m_renderBuffer.maxIndicesCount) {
                afterRender();
                flushRender();
                beforeRender();
            }

            float textureIndex = 0.F;

            // texture coloring
            if(states.texture != nullptr) {
                // find our texture
                m_renderBuffer.textureSlots[1] = states.texture->getID();
            }


            for (auto it = 0; it < quadVertexSize; ++it) {
                m_renderBuffer.quadBufferPtr->Position = states.transform * m_renderBuffer.quadVertexPositions[it];
                m_renderBuffer.quadBufferPtr->Color = states.color.toGL();
                m_renderBuffer.quadBufferPtr->textureIndex = textureIndex;
                m_renderBuffer.quadBufferPtr->TextureCoords = data[it].texCoords;
                m_renderBuffer.quadBufferPtr++;
            }

            m_renderBuffer.indexCount += 6;
            m_stats.drawQuads++;
        }

        void OpenGLRender::setSize(const vec2u &size) {
            RenderImpl::setSize(size);
            init();
        }

        IntRect OpenGLRender::getViewport(const View &view) {
            float width  = static_cast<float>(m_size.x);
            float height = static_cast<float>(m_size.y);
            const FloatRect& viewport = view.getViewport();

            return IntRect(static_cast<int>(0.5F + width  * viewport.lx),
                           static_cast<int>(0.5F + height * viewport.ly),
                           static_cast<int>(0.5F + width  * viewport.width),
                           static_cast<int>(0.5F + height * viewport.height));
        }

        void OpenGLRender::applyCurrentView() {
            IntRect viewport = getViewport(m_view);
            int top = m_size.y - (viewport.ly + viewport.height);
            glViewport(viewport.lx, top, viewport.width, viewport.height);
            m_quadShader.set_parameter("projection", m_view.getTransform().get_matrix());
        }

        const View& OpenGLRender::getView() {
            return m_view;
        }

        const View& OpenGLRender::getDefaultView() {
            return m_default;
        }

        void OpenGLRender::beforeRender() const {
            memset(&m_stats, 0, sizeof(RenderStats));
            m_renderBuffer.quadBufferPtr = m_renderBuffer.quadBuffer;
        }

        void OpenGLRender::afterRender() const {
            auto size = (uint8_t*)m_renderBuffer.quadBufferPtr - (uint8_t*)m_renderBuffer.quadBuffer;
            glBindBuffer(GL_ARRAY_BUFFER, m_renderBuffer.VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, size, m_renderBuffer.quadBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        void OpenGLRender::flushRender() const {
            // flush logic
//            for(int it = 0; it < m_renderBuffer.textureSlots.size(); ++it)
//                glBindTexture(it, m_renderBuffer.textureSlots[it]);

            glActiveTexture(GL_TEXTURE0);
            if(m_renderBuffer.textureSlots[1] != 0 ) {
                glBindTexture(GL_TEXTURE_2D, m_renderBuffer.textureSlots[1]);
            } else {
                glBindTexture(GL_TEXTURE_2D, m_renderBuffer.textureSlots[0]);
            }


            glBindVertexArray(m_renderBuffer.VAO);
            glDrawElements(GL_TRIANGLES, m_renderBuffer.indexCount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);

            m_renderBuffer.indexCount = 0;
            m_renderBuffer.textureSlotIndex = 1;
            m_stats.drawCalls++;
            m_renderBuffer.textureSlots[1] = 0;
        }

        const RenderStats &OpenGLRender::getStats() const {
            return m_stats;
        }

    }
}