//
// Created by Necromant on 01.10.2021.
//

#pragma once
#include <robot2D/Config.hpp>

namespace robot2D {

    /// \brief Provides current BatchRender statistics
    struct ROBOT2D_EXPORT_API RenderStats final {
        /// GPU draw calls since beforeRender(), including custom geometry.
        unsigned drawCalls;

        /// Submitted quads since beforeRender(); visibility is not tested here.
        unsigned drawQuads;

        /// drawQuads * 4 ( quad = 4 vertices)
        unsigned drawVertices;
    };

}
