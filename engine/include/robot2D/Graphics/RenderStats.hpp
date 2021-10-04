//
// Created by Necromant on 01.10.2021.
//

#pragma once
#include <robot2D/Config.hpp>

namespace robot2D {
    struct ROBOT2D_EXPORT_API RenderStats final {
        unsigned drawCalls;
        unsigned drawQuads;
        unsigned drawVertices;
    };

}
