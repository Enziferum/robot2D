//
// Created by Necromant on 29.09.2021.
//

#pragma once
#include <vector>
#include <robot2D/Core/Vector2.hpp>
#include "Color.hpp"

namespace robot2D {
    struct Vertex {
        vec2f position;
        vec2f texCoords;
        Color color;
    };

    using VertexData = std::vector<Vertex>;
}