#pragma once

#include <vector>

#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Graphics/View.hpp>

#include "RenderBuffer.hpp"

namespace robot2D {
    namespace priv {

        struct VertexArrayCache {
            VertexArray::Ptr m_vertexArray;
            RenderStates m_states;
            std::size_t m_batchesBefore = 0;
        };

        // TODO: @a.raag Maybe move out layer creation from Render?
        struct RenderLayer {
            RenderLayer() = default;
            RenderLayer(const RenderLayer&) = delete;
            RenderLayer& operator=(const RenderLayer&) = delete;
            RenderLayer(RenderLayer&&) noexcept = default;
            RenderLayer& operator=(RenderLayer&&) noexcept = default;
            ~RenderLayer() = default;

            RenderBuffer m_renderBuffer;
            ShaderHandler m_quadShader;
            robot2D::View m_view;

            std::vector<VertexArrayCache> m_vertexArrayCache;
        };
    }
}
