#include <robot2D/Graphics/RenderAPI.hpp>

namespace robot2D {
    RenderApi RenderAPI::m_api = RenderApi::OpenGL3_3;
    RenderDimensionType RenderAPI::m_dimensionType = RenderDimensionType::TwoD;

    RenderApi& RenderAPI::getOpenGLVersion() {
        return m_api;
    }

    RenderDimensionType& RenderAPI::getDimensionType() {
        return m_dimensionType;
    }
}