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
#include <cassert>

#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Buffer.hpp>
#include <robot2D/Graphics/VertexArray.hpp>
#include <robot2D/Util/Logger.hpp>
#include "OpenGLRender.hpp"

namespace robot2D {
    namespace priv {
#define ROBOT2D_DEBUG
        const std::string vertexSource = R"(
            #version 330 core
            layout (location = 0) in vec2 position;
            layout (location = 1) in vec4 color;
            layout (location = 2) in vec2 textureCoords;
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
            layout (location = 0) out vec4 fragColor;
            in vec2 TexCoords;
            in vec4 Color;
            in float TexIndex;
            uniform sampler2D textureSamplers[16];
            void main()
            {
                int index = int(TexIndex);
                fragColor = texture(textureSamplers[index], TexCoords) * Color;
            }
        )";

        constexpr short quadVertexSize = 4;
        constexpr short maxTextureSlots = 16;
#ifdef ROBOT2D_WINDOWS
        void OpenGLMessageCallback(
                unsigned source,
                unsigned type,
                unsigned id,
                unsigned severity,
                int length,
                const char* message,
                const void* userParam)
        {
            switch (severity)
            {
                case GL_DEBUG_SEVERITY_HIGH:         RB_CORE_CRITICAL(message); return;
                case GL_DEBUG_SEVERITY_MEDIUM:       RB_CORE_ERROR(message); return;
                case GL_DEBUG_SEVERITY_LOW:          RB_CORE_WARN(message); return;
                case GL_DEBUG_SEVERITY_NOTIFICATION: RB_CORE_TRACE(message); return;
            }

            assert(false && "Unknown severity level!");
        }
#endif
        OpenGLRender::OpenGLRender() {}

        OpenGLRender::~OpenGLRender() {
            destroy();
        }

        void OpenGLRender::initOpenGL() {


#ifdef ROBOT2D_WINDOWS
            RB_CORE_INFO("{0}", __FUNCTION__ );
#endif
            RB_CORE_INFO("OpenGL Info:");
            RB_CORE_INFO("Vendor: {0} ", glGetString(GL_VENDOR));
            RB_CORE_INFO("Renderer: {0}", glGetString(GL_RENDERER));
            RB_CORE_INFO("Version: {0}", glGetString(GL_VERSION));
            
#ifdef ROBOT2D_WINDOWS
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(OpenGLMessageCallback, nullptr);

            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif
            glEnable(GL_BLEND); // blending function
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        void OpenGLRender::init() {
            initOpenGL();

            m_renderBuffer.quadBuffer = new RenderVertex[m_renderBuffer.maxQuadsCount];

            m_renderBuffer.vertexArray = VertexArray::Create();
            m_renderBuffer.vertexBuffer = VertexBuffer::Create(sizeof(RenderVertex) * m_renderBuffer.maxQuadsCount);
            // for OpenGL name - utility only
            m_renderBuffer.vertexBuffer -> setAttributeLayout({
                    {ElementType::Float2, "Position"},
                    {ElementType::Float4, "Color(RGBA)"},
                    {ElementType::Float2, "TextureCoords"},
                    {ElementType::Float1, "TextureIndex"}
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
            m_renderBuffer.vertexArray -> setIndexBuffer(indexBuffer);

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

            GLubyte texData[] = { 255, 255, 255, 255 }; // 0xffffffff;

            m_renderBuffer.whiteTexture.create({1, 1}, texData);
            m_renderBuffer.textureSlots[0] = m_renderBuffer.whiteTexture.getID();

            int samples[maxTextureSlots];
            for(int it = 0; it < maxTextureSlots; ++it)
                samples[it] = it;

            m_quadShader.use();
            m_quadShader.set_parameter("textureSamplers", samples, maxTextureSlots);

            for(int it = 1; it < maxTextureSlots; ++it)
                m_renderBuffer.textureSlots[it] = 0;

            m_default.reset(FloatRect(0.F, 0.F,
                                      static_cast<float>(m_size.x),
                                      static_cast<float>(m_size.y)));
            m_view = m_default;
            m_quadShader.set_parameter("projection", m_view.getTransform().get_matrix());
            m_quadShader.unUse();
        }


        void OpenGLRender::destroy() {
            delete[] m_renderBuffer.quadBuffer;
        }

        void OpenGLRender::setView(const View& view) {
            m_view = view;
            applyCurrentView();
        }

        void OpenGLRender::clear(const Color& color) {
            auto glColor = color.toGL();
            glClearColor(glColor.red, glColor.green, glColor.blue, glColor.alpha);
            glClear(GL_COLOR_BUFFER_BIT);
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
                    //if (m_renderBuffer.textureSlotIndex >= maxTextureSlots)
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
                m_renderBuffer.quadBufferPtr->color = states.color.toGL();
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
            m_quadShader.use();
            m_quadShader.set_parameter("projection", m_view.getTransform().get_matrix());
            m_quadShader.unUse();
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
            uint32_t size = uint32_t((uint8_t*)m_renderBuffer.quadBufferPtr - (uint8_t*)m_renderBuffer.quadBuffer);
            m_renderBuffer.vertexBuffer -> setData(m_renderBuffer.quadBuffer, size);
        }

        void OpenGLRender::flushRender() const {
            for(int it = 0; it < m_renderBuffer.textureSlotIndex; ++it) {
#ifdef ROBOT2D_MACOS
                glActiveTexture(GL_TEXTURE0+it);
                glBindTexture(GL_TEXTURE_2D, m_renderBuffer.textureSlots[it]);
#elif ROBOT2D_WINDOWS
                glBindTextureUnit(it, m_renderBuffer.textureSlots[it]);
#endif
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