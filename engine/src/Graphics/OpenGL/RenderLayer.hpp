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
        };

        // Maybe move out layer creation from Render
        struct RenderLayer {
            RenderLayer() = default;
            ~RenderLayer();

            void destroy();

            RenderBuffer m_renderBuffer;
            ShaderHandler m_quadShader;
            robot2D::View m_view;

            std::vector<VertexArrayCache> m_vertexArrayCache;
        };
    }
}