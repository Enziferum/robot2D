//
// Created by Necromant on 29.09.2021.
//

#pragma once
#include <vector>
#include <robot2D/Core/Vector2.hpp>
#include "Color.hpp"

namespace robot2D {
    // in version 0.2 - 0.3 using only textureCoords for Quads
    struct Vertex {
        vec2f position;
        vec2f texCoords;
        Color color;
    };

    using VertexData = std::vector<Vertex>;
}