#pragma once

#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Graphics/View.hpp>

#include "RenderBuffer.hpp"

namespace robot2D {
    // Maybe move out layer creation from Render
    struct RenderLayer {
        RenderLayer() = default;
        ~RenderLayer();

        void destroy();

        RenderBuffer m_renderBuffer;
        ShaderHandler m_quadShader;
        robot2D::View m_view;
    };
}