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


namespace robot2D::priv {

    struct Viewport { int x, y, w, h, scale; };

    inline Viewport ComputeIntegerViewport(int winW, int winH, int virtW, int virtH) {
        float sx = float(winW) / virtW;
        float sy = float(winH) / virtH;
        int scale = int(std::floor(std::min(sx, sy)));
        if(scale < 1)
            scale = 1;

        int w = virtW * scale;
        int h = virtH * scale;
        int x = (winW - w) / 2;
        int y = (winH - h) / 2;         // letterbox
        return { x, y, w, h, scale };
    }


    struct VirtualSize { int virtW, virtH, scale, viewportX, viewportY, viewportW, viewportH; };

    VirtualSize chooseVirtualSize(int winW, int winH, int aspectNum, int aspectDen, bool integerScale = true) {
        double targetW = winW;
        double targetH = (double)winW * aspectDen / aspectNum;
        if(targetH > winH) {
            targetH = winH;
            targetW = (double)winH * aspectNum / aspectDen;
        }

        int vpW = std::max(1, (int)std::floor(targetW));
        int vpH = std::max(1, (int)std::floor(targetH));
        int vpX = (winW - vpW) / 2;
        int vpY = (winH - vpH) / 2;

        int scale = 1;
        int virtW = vpW;
        int virtH = vpH;

        if(integerScale) {
            int sW = vpW / aspectNum;
            int sH = vpH / aspectDen;
            scale = std::max(1, std::min(sW, sH));
            virtW = aspectNum * scale;
            virtH = aspectDen * scale;

            vpW = virtW;
            vpH = virtH;
            vpX = (winW - vpW) / 2;
            vpY = (winH - vpH) / 2;
        }
        else {
            scale = 0;
        }

        return { virtW, virtH, scale, vpX, vpY, vpW, vpH };
    }

    ///////////////////// Consts /////////////////////
    constexpr short quadVertexSize = 4;
    constexpr short maxTextureSlots = RenderBuffer::maxTextureSlots;
    constexpr unsigned int defaultLayerID = 1;
    constexpr unsigned int maxLayers = 5;
    constexpr unsigned int defaultLayersValue = 2;
    ///////////////////// Consts /////////////////////

    // TODO from RenderAPI ?
    constexpr bool useBlending = true;

    const robot2D::vec2f textureCoords[4] = { { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f } };


    struct RenderCache {
        /// TODO(a.raag): Primary idea to be possible render vertexarray + quads in begin end render cycle.
    };

    OpenGLRender::OpenGLRender(const vec2u& windowSize) {
        m_size = windowSize;
        m_surfaceSize = windowSize;
        m_drawViewport = {0, 0, static_cast<int>(windowSize.x), static_cast<int>(windowSize.y)};

        m_shaderKeys.insert({ShaderKey::TextureSamples, "textureSamplers"});
        m_shaderKeys.insert({ShaderKey::Projection, "projection"});
        m_shaderKeys.insert({ShaderKey::is3DRender, "is3DRender"});
        m_shaderKeys.insert({ShaderKey::View, "view"});
    }

    OpenGLRender::~OpenGLRender() {
        destroy();
    }

    void OpenGLRender::setupOpenGL() {
        configureGLDiagnostics();
        logGLContextInfo();
        m_renderApi = RenderAPI::getOpenGLVersion();
        m_dimensionType = RenderAPI::getDimensionType();

        if(useBlending) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        if(m_dimensionType != RenderDimensionType::TwoD)
            glEnable(GL_DEPTH_TEST);
    }

    void OpenGLRender::createLayer() {
        if(m_renderLayers.size() < maxLayers)
            setupLayer();
    }

    void OpenGLRender::setupLayer() {
        RenderLayer renderLayer;
        RenderBuffer& m_renderBuffer = renderLayer.m_renderBuffer;
        ShaderHandler& m_quadShader = renderLayer.m_quadShader;

        m_renderBuffer.quadBuffer = std::make_unique<RenderVertex[]>(m_renderBuffer.maxVerticesCount);
        m_renderBuffer.quadBufferPtr = m_renderBuffer.quadBuffer.get();
        m_renderBuffer.vertexArray = VertexArray::Create();
        m_renderBuffer.vertexBuffer = VertexBuffer::Create(sizeof(RenderVertex) * m_renderBuffer.maxVerticesCount);
        // for OpenGL name - utility only
        m_renderBuffer.vertexBuffer -> setAttributeLayout({
            {ElementType::Float3, "Position"},
            {ElementType::Float4, "Color(RGBA)"},
            {ElementType::Float2, "TextureCoords"},
            {ElementType::Float1, "TextureIndex"},
            {ElementType::Int1, "EntityID"},
        });
        m_renderBuffer.vertexArray -> setVertexBuffer(m_renderBuffer.vertexBuffer);

        auto quadIndices = new uint32_t[m_renderBuffer.maxIndicesCount];

        uint32_t offset = 0;
        for(uint32_t i = 0; i < m_renderBuffer.maxIndicesCount; i += 6) {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }
        auto indexBuffer = IndexBuffer::Create(quadIndices,
                m_renderBuffer.maxIndicesCount * sizeof(uint32_t));
        m_renderBuffer.vertexArray -> setIndexBuffer(indexBuffer);

        delete[] quadIndices;

        m_renderBuffer.quadVertexPositions[0] = {0.F, 0.F, 0.F};
        m_renderBuffer.quadVertexPositions[1] = {1.F, 0.F, 0.F};
        m_renderBuffer.quadVertexPositions[2] = {1.F, 1.F, 0.F};
        m_renderBuffer.quadVertexPositions[3] = {0.F, 1.F, 0.F};

        std::string openGLVersion;
        if(m_renderApi == RenderApi::OpenGL4_3)
            openGLVersion = "#version 430 core \n";
        else if(m_renderApi == RenderApi::OpenGL4_5)
            openGLVersion = "#version 450 core \n";

        auto openGLVertexSource = openGLVersion + OpenGL::vertexSource;
        auto openGLFragmentSource = openGLVersion + OpenGL::fragmentSource;

        if(!m_quadShader.createShader(ShaderType::Vertex,
            openGLVertexSource, false, "Core.Quad.vertex")) {
            std::string reason = "Can't load Quad Vertex Shader";
            throw std::runtime_error(reason);
        }

        if(!m_quadShader.createShader(ShaderType::Fragment,
            openGLFragmentSource, false, "Core.Quad.fragment")) {
            std::string reason = "Can't load Quad Fragment Shader";
            throw std::runtime_error(reason);
        }

        GLubyte texData[] = { 255, 255, 255, 255 }; // 0xffffffff;

        m_renderBuffer.whiteTexture -> create({1, 1}, texData);
        m_renderBuffer.textureSlots[0] = m_renderBuffer.whiteTexture -> getID();

        int samples[maxTextureSlots];
        for(int it = 0; it < maxTextureSlots; ++it)
            samples[it] = it;

        m_quadShader.use();
        m_quadShader.setArray(m_shaderKeys[ShaderKey::TextureSamples],
            samples, maxTextureSlots);

        for(int it = 1; it < maxTextureSlots; ++it)
            m_renderBuffer.textureSlots[it] = 0;

        renderLayer.m_view = m_default;
        m_quadShader.setMatrix(m_shaderKeys[ShaderKey::Projection],
            renderLayer.m_view.getTransform().get_matrix());
        if(m_dimensionType == RenderDimensionType::TwoD)
            m_quadShader.set(m_shaderKeys[ShaderKey::is3DRender], false);
        else
            m_quadShader.set(m_shaderKeys[ShaderKey::is3DRender], true);
        m_quadShader.setMatrix(m_shaderKeys[ShaderKey::View], Matrix3D{}.getRaw());
        m_quadShader.unUse();

        m_renderLayers.emplace_back(std::move(renderLayer));
    }

    void OpenGLRender::setup() {
        setupOpenGL();

        m_default.reset(FloatRect(0.F, 0.F,
            static_cast<float>(m_size.x),
            static_cast<float>(m_size.y)));

        for(unsigned int it = 0; it < defaultLayersValue; ++it)
            createLayer();
    }

    void OpenGLRender::destroy() {
        m_renderLayers.clear();
    }

    namespace {
        bool sameView(const View& left, const View& right) {
            const auto& a = left.getViewport();
            const auto& b = right.getViewport();
            const auto& ar = left.getRectangle();
            const auto& br = right.getRectangle();
            return left.getTransform() == right.getTransform() && left.isClipping() == right.isClipping()
                   && a.lx == b.lx && a.ly == b.ly && a.width == b.width && a.height == b.height
                   && ar.lx == br.lx && ar.ly == br.ly && ar.width == br.width && ar.height == br.height;
        }
    }

    void OpenGLRender::setView(const View& view, unsigned int layerID) {
        if(m_renderLayers.size() <= layerID)
            layerID = m_renderLayers.size() - 1;
        auto& layer = m_renderLayers[layerID];
        if(layer.m_customProjection || !sameView(layer.m_view, view))
            afterRender();
        layer.m_view = view;
        layer.m_customProjection = false;
        applyCurrentView(layerID);
    }

    void OpenGLRender::setSize(const vec2u& size) {
        if(!size.x || !size.y || size == m_size)
            return;
        afterRender();
        const auto previousDefault = m_default;
        m_size = size;
        m_surfaceSize = size;
        m_drawViewport = {0, 0, static_cast<int>(size.x), static_cast<int>(size.y)};
        m_virtualViewport = {};
        m_default.reset({0, 0, static_cast<float>(size.x), static_cast<float>(size.y)});
        for(unsigned int layerID = 0; layerID < m_renderLayers.size(); ++layerID) {
            auto& layer = m_renderLayers[layerID];
            if(!layer.m_customProjection && sameView(layer.m_view, previousDefault)) {
                layer.m_view = m_default;
                applyCurrentView(layerID);
            }
        }
        glViewport(0, 0, size.x, size.y);
    }

    void OpenGLRender::setViewVirtual(const vec2u& windowSize, const View& view, unsigned int layerID) {
        if(!windowSize.x || !windowSize.y)
            return;
        if(layerID >= m_renderLayers.size())
            layerID = m_renderLayers.size() - 1;
        const auto virtualSize = chooseVirtualSize(windowSize.x, windowSize.y, 16, 9,
                windowSize.x >= 16 && windowSize.y >= 9);
        const std::array<int, 4> viewport = {virtualSize.viewportX, virtualSize.viewportY,
                                             virtualSize.viewportW, virtualSize.viewportH};
        auto& layer = m_renderLayers[layerID];
        if(layer.m_customProjection || !sameView(layer.m_view, view)
            || viewport != m_drawViewport || windowSize != m_surfaceSize)
            afterRender();
        m_surfaceSize = windowSize;
        m_drawViewport = viewport;
        m_virtualViewport = viewport;
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        layer.m_view = view;
        layer.m_customProjection = false;
        applyCurrentView(layerID);
    }

    IntRect OpenGLRender::getViewport(const View& view) {
        return static_cast<const OpenGLRender&>(*this).getViewport(view);
    }

    IntRect OpenGLRender::getViewport(const View& view) const {
        const auto& viewport = view.getViewport();
        const float width = static_cast<float>(m_drawViewport[2]);
        const float height = static_cast<float>(m_drawViewport[3]);
        const int top = static_cast<int>(m_surfaceSize.y) - m_drawViewport[1] - m_drawViewport[3];
        return {m_drawViewport[0] + static_cast<int>(0.5f + width * viewport.lx),
                top + static_cast<int>(0.5f + height * viewport.ly),
                static_cast<int>(0.5f + width * viewport.width),
                static_cast<int>(0.5f + height * viewport.height)};
    }

    void OpenGLRender::applyCurrentView(unsigned int layerID) {
        auto& m_view = m_renderLayers[layerID].m_view;
        auto& m_quadShader = m_renderLayers[layerID].m_quadShader;

        if(m_view.isClipping())
            return;

        m_quadShader.use();
        m_quadShader.setMatrix(m_shaderKeys[ShaderKey::Projection], m_view.getTransform().get_matrix());
        m_quadShader.set(m_shaderKeys[ShaderKey::is3DRender], false);
        m_quadShader.unUse();
    }

    void OpenGLRender::setView3D(const Matrix3D& projection, const Matrix3D& view) {
        afterRender();
        m_renderLayers[defaultLayerID].m_customProjection = true;
        assert(m_dimensionType != RenderDimensionType::TwoD && "To use 3D view function set RenderDimension "
            "Mode to 3D / Both Mode");
        auto& m_quadShader = m_renderLayers[defaultLayerID].m_quadShader;
        m_quadShader.use();
        m_quadShader.setMatrix(m_shaderKeys[ShaderKey::Projection], projection.getRaw());
        m_quadShader.setMatrix(m_shaderKeys[ShaderKey::View], view.getRaw());
        m_quadShader.set(m_shaderKeys[ShaderKey::is3DRender], true);
        m_quadShader.unUse();
    }

    void OpenGLRender::setRawView(float* rawMatrix) {
        afterRender();
        m_renderLayers[defaultLayerID].m_customProjection = true;
        auto& m_quadShader = m_renderLayers[defaultLayerID].m_quadShader;
        m_quadShader.use();
        m_quadShader.setMatrix(m_shaderKeys[ShaderKey::Projection], rawMatrix);
        m_quadShader.unUse();
    }

    void OpenGLRender::clear(const Color& color) {
        auto glColor = color.toGL();
        glClearColor(glColor.red, glColor.green, glColor.blue, glColor.alpha);
        if(m_dimensionType != RenderDimensionType::TwoD)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        else
            glClear(GL_COLOR_BUFFER_BIT);
    }

    void OpenGLRender::clearScissor(const robot2D::UIntRect rect, const Color& color) {
        glEnable(GL_SCISSOR_TEST);
        if(m_virtualViewport[2] && m_virtualViewport[3])
            glScissor(m_virtualViewport[0], m_virtualViewport[1], m_virtualViewport[2], m_virtualViewport[3]);
        else
            glScissor(rect.lx, rect.ly, rect.width, rect.height);
        auto glColor = color.toGL();
        glClearColor(glColor.red, glColor.green, glColor.blue, glColor.alpha);
        if(m_dimensionType != RenderDimensionType::TwoD)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        else
            glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
    }

    const RenderStats& OpenGLRender::getStats() const {
        return m_stats;
    }


    unsigned int OpenGLRender::getLayerCount() const {
        return m_renderLayers.size();
    }


    const View& OpenGLRender::getView(unsigned int layerID) {
        if(m_renderLayers.size() <= layerID)
            layerID = m_renderLayers.size() - 1;
        return m_renderLayers[layerID].m_view;
    }

    const View& OpenGLRender::getDefaultView() {
        return m_default;
    }


    void OpenGLRender::resetBatch(RenderBuffer& buffer) const {
        buffer.quadBufferPtr = buffer.quadBuffer.get();
        buffer.indexCount = 0;
        buffer.textureSlotIndex = 1;
        buffer.textureSlots.fill(0);
        buffer.textureSlots[0] = buffer.whiteTexture -> getID();
    }

    void OpenGLRender::queueBatch(unsigned int layerID) const {
        auto& buffer = m_renderLayers[layerID].m_renderBuffer;
        if(buffer.indexCount == 0)
            return;
        // Keep completed batches on the CPU until this layer is executed. Drawing an
        // upper layer here would put it underneath lower layers submitted later.
        RenderBuffer::PendingBatch batch;
        batch.vertices.assign(buffer.quadBuffer.get(), buffer.quadBufferPtr);
        batch.textureSlots = buffer.textureSlots;
        batch.textureSlotCount = buffer.textureSlotIndex;
        buffer.pendingBatches.emplace_back(std::move(batch));
        resetBatch(buffer);
    }

    void OpenGLRender::beforeRender() const {
        std::array<int, 4> viewport;
        glGetIntegerv(GL_VIEWPORT, viewport.data());
        if(viewport != m_drawViewport) {
            m_drawViewport = viewport;
            m_surfaceSize = {static_cast<unsigned int>(viewport[2]), static_cast<unsigned int>(viewport[3])};
        }
        m_stats = {};
        for(auto& layer: m_renderLayers) {
            resetBatch(layer.m_renderBuffer);
            layer.m_renderBuffer.pendingBatches.clear();
            layer.m_vertexArrayCache.clear();
        }
    }

    void OpenGLRender::afterRender() const {
        for(unsigned int layerID = 0; layerID < m_renderLayers.size(); ++layerID)
            flushRender(layerID);
    }

    void OpenGLRender::drawBatch(unsigned int layerID, const RenderVertex* vertices, uint32_t vertexCount,
        const uint32_t* textureSlots, uint32_t textureSlotCount) const {
        if(vertexCount == 0)
            return;
        auto& buffer = m_renderLayers[layerID].m_renderBuffer;
        auto& shader = m_renderLayers[layerID].m_quadShader;
        buffer.vertexBuffer -> setData(vertices, vertexCount * sizeof(RenderVertex));

        for(uint32_t i = 1; i < textureSlotCount; ++i) {
            if(m_renderApi == RenderApi::OpenGL4_3) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, textureSlots[i]);
            }
            else if(m_renderApi == RenderApi::OpenGL4_5) {
#if !defined(ROBOT2D_MACOS)
                glBindTextureUnit(i, textureSlots[i]);
#endif
            }
        }
        shader.use();
        auto& view = m_renderLayers[layerID].m_view;
        if(view.isClipping()) {
            glEnable(GL_SCISSOR_TEST);
            auto rect = view.getRectangle();
            glScissor(rect.lx, rect.ly, rect.width, rect.height);
        }
        buffer.vertexArray -> Bind();
        glCall(glDrawElements, GL_TRIANGLES, static_cast<GLsizei>(vertexCount / quadVertexSize * 6),
            GL_UNSIGNED_INT, nullptr);
        buffer.vertexArray -> unBind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        shader.unUse();
        ++m_stats.drawCalls;
        if(view.isClipping())
            glDisable(GL_SCISSOR_TEST);
    }

    void OpenGLRender::flushRender(unsigned int layerID) const {
        if(layerID >= m_renderLayers.size()) {
            RB_CORE_ERROR("[Render] flush rejected: layer={} does not exist; layer-count={}",
                layerID, m_renderLayers.size());
            return;
        }
        std::array<int, 4> previousViewport;
        glGetIntegerv(GL_VIEWPORT, previousViewport.data());
        const auto viewport = getViewport(m_renderLayers[layerID].m_view);
        glViewport(viewport.lx, static_cast<int>(m_surfaceSize.y) - viewport.ly - viewport.height,
            viewport.width, viewport.height);
        auto& layer = m_renderLayers[layerID];
        auto& buffer = layer.m_renderBuffer;
        std::size_t cacheIndex = 0;
        for(std::size_t batchIndex = 0; batchIndex <= buffer.pendingBatches.size(); ++batchIndex) {
            renderCache(layerID, cacheIndex, batchIndex);
            if(batchIndex < buffer.pendingBatches.size()) {
                const auto& batch = buffer.pendingBatches[batchIndex];
                drawBatch(layerID, batch.vertices.data(), static_cast<uint32_t>(batch.vertices.size()),
                    batch.textureSlots.data(), batch.textureSlotCount);
            }
        }
        drawBatch(layerID, buffer.quadBuffer.get(),
            static_cast<uint32_t>(buffer.quadBufferPtr - buffer.quadBuffer.get()),
            buffer.textureSlots.data(), buffer.textureSlotIndex);
        buffer.pendingBatches.clear();
        layer.m_vertexArrayCache.clear();
        resetBatch(buffer);
        glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);
    }


    void OpenGLRender::render(const RenderStates& states) {
        auto layerID = states.layerID;
        if(m_renderLayers.size() <= states.layerID)
            layerID = m_renderLayers.size() - 1;

        auto& m_renderBuffer = m_renderLayers[layerID].m_renderBuffer;

        Vertex3DData quadVertexData;

        if(m_dimensionType == RenderDimensionType::ThreeD) {
            const auto& transform = states.transform3D;

            quadVertexData = {
                {
                    (transform * m_renderBuffer.quadVertexPositions[0]),
                    textureCoords[0]
                },
                {
                    (transform * m_renderBuffer.quadVertexPositions[1]),
                    textureCoords[1]
                },
                {
                    (transform * m_renderBuffer.quadVertexPositions[2]),
                    textureCoords[2]
                },
                {
                    (transform * m_renderBuffer.quadVertexPositions[3]),
                    textureCoords[3]
                }
            };
        }
        else {
            const auto& transform = states.transform;
            quadVertexData = {
                {
                    (transform * m_renderBuffer.quadVertexPositions[0].asVec2<float>()),
                    textureCoords[0]
                },
                {
                    (transform * m_renderBuffer.quadVertexPositions[1].asVec2<float>()),
                    textureCoords[1]
                },
                {
                    (transform * m_renderBuffer.quadVertexPositions[2].asVec2<float>()),
                    textureCoords[2]
                },
                {
                    (transform * m_renderBuffer.quadVertexPositions[3].asVec2<float>()),
                    textureCoords[3]
                }
            };
        }

        render(quadVertexData, states);
    }

    void OpenGLRender::render(const VertexData& data, const RenderStates& states) const {
        Vertex3DData vertex3DData;
        for(const auto& vert: data)
            vertex3DData.emplace_back(Vertex3D{vert.position, vert.texCoords, vert.color});
        render(vertex3DData, states);
    }

    void OpenGLRender::render(const Vertex3DData& data, const RenderStates& states) const {
        if(data.size() != quadVertexSize) {
            RB_CORE_ERROR("[Render] quad rejected: expected {} vertices; received {}", quadVertexSize, data.size());
            return;
        }

        int layerID = states.layerID;
        if(m_renderLayers.size() <= states.layerID)
            layerID = m_renderLayers.size() - 1;

        auto& m_renderBuffer = m_renderLayers[layerID].m_renderBuffer;

        if(m_renderBuffer.indexCount + 6 > m_renderBuffer.maxIndicesCount)
            queueBatch(layerID);

        float textureIndex = 0.F;
        if(states.texture) {
            for(uint32_t i = 1; i < m_renderBuffer.textureSlotIndex; i++) {
                if(m_renderBuffer.textureSlots[i] == states.texture -> getID()) {
                    textureIndex = static_cast<float>(i);
                    break;
                }
            }

            if(textureIndex == 0.F) {
                if(m_renderBuffer.textureSlotIndex >= maxTextureSlots) {
                    queueBatch(layerID);
                }

                textureIndex = (float)m_renderBuffer.textureSlotIndex;
                m_renderBuffer.textureSlots[m_renderBuffer.textureSlotIndex] = states.texture -> getID();
                m_renderBuffer.textureSlotIndex++;
            }
        }

        for(int i = 0; i < quadVertexSize; ++i) {
            m_renderBuffer.quadBufferPtr -> Position = data[i].position;
            m_renderBuffer.quadBufferPtr -> color = states.color.toGL();
            m_renderBuffer.quadBufferPtr -> textureIndex = textureIndex;
            m_renderBuffer.quadBufferPtr -> TextureCoords = data[i].texCoords;
            m_renderBuffer.quadBufferPtr -> entityID = states.entityID;
            m_renderBuffer.quadBufferPtr++;
        }

        m_renderBuffer.indexCount += 6;
        m_stats.drawQuads++;
        m_stats.drawVertices += quadVertexSize;
    }

    void OpenGLRender::render(const VertexArray::Ptr& vertexArray, RenderStates states) const {
        auto layerID = states.layerID;
        if(layerID >= m_renderLayers.size())
            layerID = m_renderLayers.size() - 1;

        if(!vertexArray)
            return;
        // Seal preceding quads without drawing the layer early. Remember where this
        // geometry belongs among the completed batches, including overflow batches.
        queueBatch(layerID);
        auto& layer = m_renderLayers[layerID];
        layer.m_vertexArrayCache.push_back({vertexArray, states, layer.m_renderBuffer.pendingBatches.size()});
    }

    void OpenGLRender::renderCache(unsigned int layerID, std::size_t& cacheIndex, std::size_t batchIndex) const {
        const auto& cache = m_renderLayers[layerID].m_vertexArrayCache;
        while(cacheIndex < cache.size() && cache[cacheIndex].m_batchesBefore == batchIndex) {
            const auto& it = cache[cacheIndex++];
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
            if(states.texture) {
                if(m_renderApi == RenderApi::OpenGL4_3) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, states.texture->getID());
                }
                else if(m_renderApi == RenderApi::OpenGL4_5) {
#if !defined(ROBOT2D_MACOS)
                    glBindTextureUnit(0, states.texture->getID());
#endif
                }
            }
            vertexArray -> Bind();

            GLenum drawMode = GL_TRIANGLES;
            switch(states.renderInfo.renderType) {
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
                                                                  : static_cast<GLsizei>(vertexArray -> getIndexBuffer()
                -> getSize() / 4);

            glCall(glDrawElements, drawMode,
                drawIndexCount,
                GL_UNSIGNED_INT,
                nullptr);
            ++m_stats.drawCalls;
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

    }

    void OpenGLRender::render3D(const VertexArray::Ptr& vertexArray, RenderStates states) const {
        if(!vertexArray && !states.shader)
            return;

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
        currentShader -> use();

        if(states.texture) {
            if(m_renderApi == RenderApi::OpenGL4_3) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, states.texture->getID());
            }
            else if(m_renderApi == RenderApi::OpenGL4_5) {
#if !defined(ROBOT2D_MACOS)
                glBindTextureUnit(0, states.texture->getID());
#endif
            }
        }
        vertexArray -> Bind();

        GLenum drawMode = GL_TRIANGLES;
        switch(states.renderInfo.renderType) {
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

        GLsizei drawIndexCount = states.renderInfo.indexCount ?
            static_cast<GLsizei>(states.renderInfo.indexCount):
            static_cast<GLsizei>(vertexArray -> getIndexBuffer() -> getSize() / 4);

        glCall(glDrawElements, drawMode,
            drawIndexCount,
            GL_UNSIGNED_INT,
            nullptr);
        ++m_stats.drawCalls;
        vertexArray -> unBind();
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);

        currentShader -> unUse();
        if(states.blendMode != BlendMode::None)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }


} // namespace robot2D::priv
