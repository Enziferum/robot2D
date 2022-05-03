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
#include <robot2D/Graphics/RenderAPI.hpp>
#include <robot2D/Graphics/QuadShaderTexts.hpp>

#include <robot2D/Util/Logger.hpp>
#include <robot2D/Config.hpp>

#include "OpenGLRender.hpp"


namespace robot2D {
    namespace priv {

        constexpr short quadVertexSize = 4;
        constexpr short maxTextureSlots = 16;
        constexpr unsigned int defaultLayerID = 1;
        constexpr unsigned int maxLayers = 5;

        // TODO from RenderAPI ?
        constexpr bool useBlending = true;

        const robot2D::vec2f textureCoords[4] = { { 0.0f, 0.0f },
                                           { 1.0f, 0.0f },
                                           { 1.0f, 1.0f },
                                           { 0.0f, 1.0f } };

#if defined(ROBOT2D_WINDOWS) or defined(ROBOT2D_LINUX)
        void OpenGLMessageCallback(
                unsigned source,
                unsigned type,
                unsigned id,
                unsigned severity,
                int length,
                const char* message,
                const void* userParam)
        {
            (void) source;
            (void) type;
            (void) id;
            (void) length;
            (void) userParam;
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
        OpenGLRender::OpenGLRender(const vec2u& windowSize) {
            m_size = windowSize;

            m_shaderKeys.insert({ShaderKey::TextureSamples, "textureSamplers"});
            m_shaderKeys.insert({ShaderKey::Projection, "projection"});
        }

        OpenGLRender::~OpenGLRender() {
            destroy();
        }

        void OpenGLRender::setupOpenGL() {
#if defined(ROBOT2D_WINDOWS) or defined(ROBOT2D_LINUX)
            RB_CORE_INFO("{0}", __FUNCTION__ );
#endif
            RB_CORE_INFO("OpenGL Info:");
            RB_CORE_INFO("Vendor: {0} ", glGetString(GL_VENDOR));
            RB_CORE_INFO("Renderer: {0}", glGetString(GL_RENDERER));
            RB_CORE_INFO("Version: {0}", glGetString(GL_VERSION));

            m_renderApi = RenderAPI::getOpenGLVersion();

#if defined(ROBOT2D_WINDOWS) or defined(ROBOT2D_LINUX)
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(OpenGLMessageCallback, nullptr);

            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif
            if(useBlending) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
        }

        RenderLayer::~RenderLayer() {}

        void RenderLayer::destroy() {
            delete[] m_renderBuffer.quadBuffer;
        }

        void OpenGLRender::createLayer() {
            if(m_renderLayers.size() < maxLayers)
                setupLayer();
        }

        void OpenGLRender::setupLayer() {
            RenderLayer renderLayer;
            RenderBuffer& m_renderBuffer = renderLayer.m_renderBuffer;
            ShaderHandler& m_quadShader = renderLayer.m_quadShader;

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

            auto quadIndices = new uint32_t[m_renderBuffer.maxIndicesCount];

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
            auto indexBuffer = IndexBuffer::Create(quadIndices,
                                                   m_renderBuffer.maxIndicesCount);
            m_renderBuffer.vertexArray -> setIndexBuffer(indexBuffer);

            delete[] quadIndices;

            m_renderBuffer.quadVertexPositions[0] = {0.F, 0.F};
            m_renderBuffer.quadVertexPositions[1] = {1.F, 0.F};
            m_renderBuffer.quadVertexPositions[2] = {1.F, 1.F};
            m_renderBuffer.quadVertexPositions[3] = {0.F, 1.F};

            m_renderBuffer.quadVertexPositions[0] = {-0.5F, -0.5F};
            m_renderBuffer.quadVertexPositions[1] = {0.5F, -0.5F};
            m_renderBuffer.quadVertexPositions[2] = {0.5F, 0.5F};
            m_renderBuffer.quadVertexPositions[3] = {-0.5F, 0.5F};

            std::string openGLVersion;
            if(m_renderApi == RenderApi::OpenGL3_3)
                openGLVersion = "#version 330 core \n";
            else if(m_renderApi == RenderApi::OpenGL4_5)
                openGLVersion = "#version 450 core \n";

            auto openGLVertexSource = openGLVersion + OpenGL::vertexSource;
            auto openGLFragmentSource = openGLVersion + OpenGL::fragmentSource;

            if(!m_quadShader.createShader(shaderType::vertex,
                                          openGLVertexSource, false)) {
                std::string reason = "Can't load Quad Vertex Shader";
                throw std::runtime_error(reason);
            }

            if(!m_quadShader.createShader(shaderType::fragment,
                                          openGLFragmentSource, false)) {
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
            m_quadShader.setArray(m_shaderKeys[ShaderKey::TextureSamples].c_str(),
                                  samples, maxTextureSlots);

            for(int it = 1; it < maxTextureSlots; ++it)
                m_renderBuffer.textureSlots[it] = 0;

            renderLayer.m_view = m_default;
            m_quadShader.set(m_shaderKeys[ShaderKey::Projection].c_str(),
                             renderLayer.m_view.getTransform().get_matrix());
            m_quadShader.unUse();

            m_renderLayers.emplace_back(std::move(renderLayer));
        }

        void OpenGLRender::setup() {
            setupOpenGL();

            m_default.reset(FloatRect(0.F, 0.F,
                                      static_cast<float>(m_size.x),
                                      static_cast<float>(m_size.y)));

            unsigned int defaultLayersValue = 3;
            for(unsigned int it = 0; it < defaultLayersValue; ++it)
                createLayer();
        }

        void OpenGLRender::destroy() {
            for(auto& it: m_renderLayers)
                it.destroy();
        }

        void OpenGLRender::setView(const View& view, unsigned int layerID) {
            if(m_renderLayers.size() <= layerID)
                layerID = m_renderLayers.size() - 1;
            m_renderLayers[layerID].m_view = view;
            applyCurrentView(layerID);
        }

        void OpenGLRender::clear(const Color& color) {
            auto glColor = color.toGL();
            glClearColor(glColor.red, glColor.green, glColor.blue, glColor.alpha);
            // TODO GL_DEPTH_BUFFER_BIT glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void OpenGLRender::render(const RenderStates& states) {
            auto layerID = states.layerID;
            if(m_renderLayers.size() <= states.layerID)
                layerID = m_renderLayers.size() - 1;

            auto& m_renderBuffer = m_renderLayers[layerID].m_renderBuffer;

            VertexData quadVertexData = {
                    {
                            states.transform * m_renderBuffer.quadVertexPositions[0],
                            textureCoords[0]
                    },
                    {
                            states.transform * m_renderBuffer.quadVertexPositions[1],
                            textureCoords[1]
                    },
                    {states.transform * m_renderBuffer.quadVertexPositions[2],
                            textureCoords[2]
                    },
                    {states.transform * m_renderBuffer.quadVertexPositions[3],
                            textureCoords[3]
                    }
            };
            render(quadVertexData, states);
        }

        void OpenGLRender::render(const VertexData& data, const RenderStates& states) const {
            // Rendering quads only not supported
            assert(data.size() == quadVertexSize && "Supports only Quad Vertex Data.");

            int layerID = states.layerID;
            if(m_renderLayers.size() <= states.layerID)
                layerID = m_renderLayers.size() - 1;

            auto& m_renderBuffer = m_renderLayers[layerID].m_renderBuffer;

            if(m_renderBuffer.indexCount >= m_renderBuffer.maxIndicesCount) {
                RB_CORE_INFO("INDEX COUNT > MAX, VALUE : {0}", m_renderBuffer.indexCount);
                afterRender();
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
                    if (m_renderBuffer.textureSlotIndex >= maxTextureSlots) {
                        afterRender();
                        beforeRender();
                    }

                    textureIndex = (float)m_renderBuffer.textureSlotIndex;
                    m_renderBuffer.textureSlots[m_renderBuffer.textureSlotIndex] = states.texture->getID();
                    m_renderBuffer.textureSlotIndex++;
                }
            }

            for (auto it = 0; it < quadVertexSize; ++it) {
                m_renderBuffer.quadBufferPtr->Position = data[it].position;
                m_renderBuffer.quadBufferPtr->color = states.color.toGL();
                m_renderBuffer.quadBufferPtr->textureIndex = textureIndex;
                m_renderBuffer.quadBufferPtr->TextureCoords = data[it].texCoords;
                m_renderBuffer.quadBufferPtr++;
            }

            m_renderBuffer.indexCount += 6;
            m_stats.drawQuads++;
        }

        IntRect OpenGLRender::getViewport(const View& view) {
            float width  = static_cast<float>(m_size.x);
            float height = static_cast<float>(m_size.y);
            const FloatRect& viewport = view.getViewport();

            return IntRect(static_cast<int>(0.5F + width  * viewport.lx),
                           static_cast<int>(0.5F + height * viewport.ly),
                           static_cast<int>(0.5F + width  * viewport.width),
                           static_cast<int>(0.5F + height * viewport.height));
        }

        void OpenGLRender::applyCurrentView(unsigned int layerID) {
            auto& m_view = m_renderLayers[layerID].m_view;
            IntRect viewport = getViewport(m_view);
            int top = m_size.y - (viewport.ly + viewport.height);
            (void) top;
            auto& m_quadShader = m_renderLayers[layerID].m_quadShader;

            m_quadShader.use();
            m_quadShader.set(m_shaderKeys[ShaderKey::Projection].c_str(),
                             m_view.getTransform().get_matrix());
            m_quadShader.unUse();
        }

        const View& OpenGLRender::getView(unsigned int layerID) {
            if(m_renderLayers.size() <= layerID)
                layerID = m_renderLayers.size() - 1;
            return m_renderLayers[layerID].m_view;
        }

        const View& OpenGLRender::getDefaultView() {
            return m_default;
        }

        void OpenGLRender::beforeRender() const {
            memset(&m_stats, 0, sizeof(RenderStats));
            for(auto& it: m_renderLayers)
                it.m_renderBuffer.quadBufferPtr = it.m_renderBuffer.quadBuffer;
        }

        void OpenGLRender::afterRender() const {
            unsigned int index = 0;
            for(auto& it: m_renderLayers) {
                auto size = uint32_t((uint8_t *) it.m_renderBuffer.quadBufferPtr
                                    - (uint8_t *) it.m_renderBuffer.quadBuffer);
                it.m_renderBuffer.vertexBuffer -> setData(it.m_renderBuffer.quadBuffer, size);

                //if(index == 1)
                flushRender(index);
                ++index;
            }
        }

        void OpenGLRender::flushRender(unsigned int layerID) const {
            auto& m_renderBuffer = m_renderLayers[layerID].m_renderBuffer;
            auto& m_quadShader = m_renderLayers[layerID].m_quadShader;

            for(auto it = 0; it < static_cast<int>(m_renderBuffer.textureSlotIndex); ++it) {
                if(m_renderApi == RenderApi::OpenGL3_3) {
                    glActiveTexture(GL_TEXTURE0 + it);
                    glBindTexture(GL_TEXTURE_2D, m_renderBuffer.textureSlots[it]);
                }
                else if(m_renderApi == RenderApi::OpenGL4_5) {
                #if !defined(ROBOT2D_MACOS)
                    glBindTextureUnit(it, m_renderBuffer.textureSlots[it]);
                #endif
                }
            }
            m_quadShader.use();

            m_renderBuffer.vertexArray -> Bind();
            glDrawElements(GL_TRIANGLES,
                           static_cast<GLsizei>(m_renderBuffer.indexCount),
                           GL_UNSIGNED_INT,
                           nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
            m_renderBuffer.vertexArray -> unBind();
            glActiveTexture(GL_TEXTURE0);
            m_quadShader.unUse();

            m_renderBuffer.indexCount = 0;
            m_renderBuffer.textureSlotIndex = 1;
            m_stats.drawCalls++;

            renderCache(layerID);
        }

        void OpenGLRender::render(const VertexArray::Ptr& vertexArray, RenderStates states) const {
            auto layerID = states.layerID;
            if(layerID >= m_renderLayers.size())
                layerID = m_renderLayers.size() - 1;

            m_renderLayers[layerID].m_vertexArrayCache.push_back({vertexArray, states});
        }

        void OpenGLRender::renderCache(unsigned int layerID) const {
            for(const auto& it: m_renderLayers[layerID].m_vertexArrayCache) {
                if(!it.m_vertexArray)
                    continue;

                auto& states = it.m_states;
                auto& vertexArray = it.m_vertexArray;

                switch(states.blendMode) {
                    default:
                        break;
                    case BlendMode::AlphaOne:
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                        break;
                    case BlendMode::MinusAlphaOne:
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        break;
                }

                auto currentShader = states.shader;
                if(currentShader == nullptr)
                    m_renderLayers[layerID].m_quadShader.use();
                else
                    currentShader -> use();
                if(m_renderApi == RenderApi::OpenGL3_3) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, states.texture->getID());
                }
                else if(m_renderApi == RenderApi::OpenGL4_5) {
                #if !defined(ROBOT2D_MACOS)
                    glBindTextureUnit(0, states.texture->getID());
                #endif
                }
                vertexArray -> Bind();

                GLenum drawMode = GL_TRIANGLES;
                switch (states.renderInfo.renderType) {
                    case PrimitiveRenderType::Point:
                        drawMode = GL_POINTS;
                        break;
                    case PrimitiveRenderType::Lines:
                        drawMode = GL_LINES;
                        break;
                    case PrimitiveRenderType::Triangles:
                        break;
                    case PrimitiveRenderType::Quads:
                        drawMode = GL_QUADS;
                        break;
                }

                GLsizei drawIndexCount = states.renderInfo.indexCount ? static_cast<GLsizei>(states.renderInfo.indexCount)
                                                           : static_cast<GLsizei>(vertexArray -> getIndexBuffer() -> getSize() / 4);

                glDrawElements(drawMode,
                               drawIndexCount,
                               GL_UNSIGNED_INT,
                               nullptr);
                vertexArray -> unBind();
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE0);

                if(currentShader == nullptr)
                    m_renderLayers[layerID].m_quadShader.unUse();
                else
                    currentShader -> unUse();

                if(states.blendMode != BlendMode::None)
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }

            m_renderLayers[layerID].m_vertexArrayCache.clear();
        }

        const RenderStats& OpenGLRender::getStats() const {
            return m_stats;
        }

        unsigned int OpenGLRender::getLayerCount() const {
            return m_renderLayers.size();
        }

    }
}