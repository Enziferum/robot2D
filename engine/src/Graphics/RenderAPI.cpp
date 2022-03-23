#include <robot2D/Graphics/RenderAPI.hpp>

namespace robot2D {
    RenderApi RenderAPI::m_api = RenderApi::OpenGL3_3;

    RenderApi& RenderAPI::getOpenGLVersion() {
        return m_api;
    }
}