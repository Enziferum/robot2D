#pragma once

namespace robot2D {
    enum class RenderApi {
        OpenGL4_3,
        OpenGL4_5
    };

    enum class RenderDimensionType {
        TwoD,
        ThreeD,
        Both
    };

    /**
     * \brief Contains information about RenderAPI
     */
    class RenderAPI {
    public:
        static RenderApi& getOpenGLVersion();
        static RenderDimensionType& getDimensionType();
    private:
        friend class RenderTarget;
        static RenderApi m_api;
        static RenderDimensionType m_dimensionType;
    };
}