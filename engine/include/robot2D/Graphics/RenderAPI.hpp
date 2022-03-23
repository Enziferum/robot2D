#pragma once

namespace robot2D {
    enum class RenderApi {
        OpenGL3_3,
        OpenGL4_5
    };

    /**
     * \brief Contains information about RenderAPI
     */
    class RenderAPI {
    public:
        static RenderApi& getOpenGLVersion();
    private:
        friend class RenderTarget;
        static RenderApi m_api;
    };
}