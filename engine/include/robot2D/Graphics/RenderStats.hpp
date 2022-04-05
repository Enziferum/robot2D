//
// Created by Necromant on 01.10.2021.
//

#pragma once
#include <robot2D/Config.hpp>

namespace robot2D {

    /// \brief Provides current BatchRender statistics
    struct ROBOT2D_EXPORT_API RenderStats final {
        /// How many time your GPU should process input buffer.
        unsigned drawCalls;

        /// How many visible! quads onto screen.
        unsigned drawQuads;

        /// drawQuads * 4 ( quad = 4 vertices)
        unsigned drawVertices;
    };

}
