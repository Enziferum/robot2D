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

#include "OpenGLRender.hpp"

namespace robot2D {
    namespace priv {

        std::string vertexSource = R"(

        )";

        std::string fragmentSource = R"(

        )";

        const char* vertexShaderSource = "#version 330 core\n"
                                         "layout (location = 0) in vec2 position;\n"
                                         "layout (location = 1) in vec2 textureCoords;\n"
                                         "layout (location = 2) in vec4 color;\n"
                                         "layout (location = 3) in float textureIndex;\n"
                                         "out vec2 TexCoords; \n"
                                         "out vec4 Color; \n"
                                         "out float TexIndex; \n"
                                         "uniform mat4 projection; \n"
                                         "void main()\n"
                                         "{\n"
                                         "   TexCoords = textureCoords; \n"
                                         "   Color = color; \n"
                                         "   TexIndex = textureIndex; \n"
                                         "   gl_Position = projection * vec4(position, 0.0, 1.0); \n"
                                         "}\0";

        const char* fragmentShaderSource = "#version 330 core \n"
                                           "in vec2 TexCoords; \n"
                                           "in vec4 Color; \n"
                                           "in float TexIndex; \n"
                                           "out vec4 fragColor; \n"
                                           "uniform sampler2D sprite; \n"
                                           "void main()\n"
                                           "{\n"
                                           "  fragColor = Color * texture(sprite, TexCoords);\n"
                                           "}\0";


        constexpr short quadVertexSize = 4;


        OpenGLRender::OpenGLRender() {}

        OpenGLRender::~OpenGLRender() {
            destroy();
        }

        void OpenGLRender::init() {
            m_renderBuffer.quadBuffer = new RenderVertex[m_renderBuffer.maxQuadsCount];

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
            delete[] quadIndices;


            m_renderBuffer.quadVertexPositions[0] = {-0.5f, -0.5f};
            m_renderBuffer.quadVertexPositions[1] = {0.5f, -0.5f};
            m_renderBuffer.quadVertexPositions[2] = {0.5f, 0.5f};
            m_renderBuffer.quadVertexPositions[3] = {-0.5f, 0.5f};

            if(!m_quadShader.createShader(shaderType::vertex,
                                             vertexShaderSource, false)) {
                std::string reason = "Can't load Quad Vertex Shader";
                throw std::runtime_error(reason);
            }

            if(!m_quadShader.createShader(shaderType::fragment,
                                             fragmentShaderSource, false)) {
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
            for(int it = 1; it < 32; ++it)
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

            float textureIndex = 0.f;

            // texture coloring
            if(states.texture != nullptr) {
                // find our texture
                m_renderBuffer.textureSlots[1] = states.texture->getID();
            } else {
                m_renderBuffer.textureSlots[1] = 0;
            }


            for (auto it = 0; it < quadVertexSize; ++it) {
                m_renderBuffer.quadBufferPtr->Position = states.transform * m_renderBuffer.quadVertexPositions[it];
                float r = states.color.r / 255;
                float g = states.color.g / 255;
                float b = states.color.b / 255;
                float a = 1;
                m_renderBuffer.quadBufferPtr->Color = {r, g, b, a};
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

            return IntRect(static_cast<int>(0.5f + width  * viewport.lx),
                           static_cast<int>(0.5f + height * viewport.ly),
                           static_cast<int>(0.5f + width  * viewport.width),
                           static_cast<int>(0.5f + height * viewport.height));
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

            if(m_renderBuffer.textureSlots[1] != 0 ) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_renderBuffer.textureSlots[1]);
            }


            glBindVertexArray(m_renderBuffer.VAO);
            glDrawElements(GL_TRIANGLES, m_renderBuffer.indexCount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);

            m_renderBuffer.indexCount = 0;
            m_renderBuffer.textureSlotIndex = 1;
            m_stats.drawCalls++;
        }

        const RenderStats &OpenGLRender::getStats() const {
            return m_stats;
        }

    }
}