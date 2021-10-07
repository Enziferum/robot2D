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
#include <robot2D/Graphics/VertexArray.hpp>
#include <robot2D/Util/Logger.hpp>

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
            out vec4 fragColor;
            uniform sampler2D textureSamplers[32];
            void main()
            {
                int index = int(TexIndex);
                fragColor = texture(textureSamplers[index], TexCoords) * Color;
            }
        )";

        constexpr short quadVertexSize = 4;


        OpenGLRender::OpenGLRender() {}

        OpenGLRender::~OpenGLRender() {
            destroy();
        }

        void OpenGLRender::init() {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            m_renderBuffer.quadBuffer = new RenderVertex[m_renderBuffer.maxQuadsCount];

            m_renderBuffer.vertexArray = VertexArray::Create();
            m_renderBuffer.vertexBuffer = VertexBuffer::Create(sizeof(RenderVertex) * m_renderBuffer.maxQuadsCount);
            m_renderBuffer.vertexBuffer -> setAttributeLayout({
                    {ElementType::Float2, ""},
                    {ElementType::Float2, ""},
                    {ElementType::Float4, ""},
                    {ElementType::Float1, ""}
            });
            m_renderBuffer.vertexArray -> setVertexBuffer(m_renderBuffer.vertexBuffer);

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
            auto indexBuffer = IndexBuffer::Create(quadIndices, m_renderBuffer.maxIndicesCount);
            m_renderBuffer.vertexArray ->setIndexBuffer(indexBuffer);

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

            glGenTextures(1, &m_renderBuffer.whiteTexture);
            glBindTexture(GL_TEXTURE_2D, m_renderBuffer.whiteTexture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


            //uint32_t textureData = 0xffffffff;
            GLubyte texData[] = { 255, 255, 255, 255 };
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);

            m_renderBuffer.textureSlots[0] = m_renderBuffer.whiteTexture;

            m_quadShader.use();

            int samples[32];
            for(int it = 0; it < 32; ++it)
                samples[it] = it;

            m_quadShader.set_parameter("textureSamplers", samples, 32);

            for(int it = 1; it < 32; ++it)
                m_renderBuffer.textureSlots[it] = 0;

            m_default.reset(FloatRect(0.F, 0.F,
                                      static_cast<float>(m_size.x),
                                      static_cast<float>(m_size.y)));
            m_view = m_default;
            m_quadShader.set_parameter("projection", m_view.getTransform().get_matrix());
            m_quadShader.unUse();

        }


        void OpenGLRender::destroy() {
            glDeleteTextures(1, &m_renderBuffer.whiteTexture);
            delete[] m_renderBuffer.quadBuffer;
        }


        void OpenGLRender::setView(const View& view) {
            m_view = view;
            applyCurrentView();
        }

        void OpenGLRender::render(const RenderStates& states) {
            robot2D::vec2f textureCoords[] = { { 0.0f, 0.0f },
                                               { 1.0f, 0.0f },
                                               { 1.0f, 1.0f },
                                               { 0.0f, 1.0f } };
            render({{{}, textureCoords[0], robot2D::Color::White},
                         {{}, textureCoords[1], robot2D::Color::White},
                         {{}, textureCoords[2], robot2D::Color::White},
                         {{}, textureCoords[3], robot2D::Color::White}}, states);
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

            if(states.texture) {
                for (uint32_t it = 1; it < m_renderBuffer.textureSlotIndex; it++)
                {
                    if (m_renderBuffer.textureSlots[it] == states.texture->getID())
                    {
                        textureIndex = (float)it;
                        break;
                    }
                }

                if (textureIndex == 0.F)
                {
//                if (m_renderBuffer.textureSlotIndex >= 32)
//                    NextBatch();

                    textureIndex = (float)m_renderBuffer.textureSlotIndex;
                    m_renderBuffer.textureSlots[m_renderBuffer.textureSlotIndex] = states.texture->getID();
                    m_renderBuffer.textureSlotIndex++;
                }
            }

            robot2D::vec2f textureCoords[] = { { 0.0f, 0.0f },
                                               { 1.0f, 0.0f },
                                               { 1.0f, 1.0f },
                                               { 0.0f, 1.0f } };

            for (auto it = 0; it < quadVertexSize; ++it) {
                m_renderBuffer.quadBufferPtr->Position = states.transform * m_renderBuffer.quadVertexPositions[it];
                m_renderBuffer.quadBufferPtr->Color = states.color.toGL();

                m_renderBuffer.quadBufferPtr->textureIndex = textureIndex;
                //data[it].texCoords
                m_renderBuffer.quadBufferPtr->TextureCoords = textureCoords[it];
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
            glViewport(viewport.lx, top, viewport.width, viewport.height);\
            m_quadShader.use();
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
            m_renderBuffer.vertexBuffer -> setData(m_renderBuffer.quadBuffer, size);
        }

        void OpenGLRender::flushRender() const {
            // flush logic

            for(int it = 0; it < m_renderBuffer.textureSlotIndex; ++it) {
                glActiveTexture(GL_TEXTURE0+it);
                glBindTexture(GL_TEXTURE_2D, m_renderBuffer.textureSlots[it]);
            }
            m_quadShader.use();

            m_renderBuffer.vertexArray -> Bind();
            glDrawElements(GL_TRIANGLES, m_renderBuffer.indexCount, GL_UNSIGNED_INT, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
            m_renderBuffer.vertexArray -> unBind();
            glActiveTexture(GL_TEXTURE0);
            m_quadShader.unUse();

            m_renderBuffer.indexCount = 0;
            m_renderBuffer.textureSlotIndex = 1;
            m_stats.drawCalls++;
        }

        const RenderStats& OpenGLRender::getStats() const {
            return m_stats;
        }
    }
}