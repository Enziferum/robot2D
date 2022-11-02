/*********************************************************************
(c) Alex Raag 2021
https://github.com/Enziferum
robot2D - Zlib license.
This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#pragma once
#include <vector>
#include <robot2D/Core/Vector2.hpp>
#include <robot2D/Core/Vector3.hpp>
#include "Color.hpp"

namespace robot2D {
    /**
     * \brief Describe input blob buffer.
     * \details Input buffer contains information what to render onto screen. \n
     * Vertex provides information for each part of shape. \n
     * Quad has 4 vertices. \n
     * Triangle has 3 vertices.
     */
    struct ROBOT2D_EXPORT_API Vertex {
        /// Position of vertex without any transformation by default.
        mutable vec2f position;
        /// Texture coordinates. Values from 0.0 to 1.0.
        mutable vec2f texCoords;
        /// Color
        mutable Color color = Color::White;
    };

    using VertexData = std::vector<Vertex>;

    /**
     * \brief Describe input blob buffer.
     * \details Input buffer contains information what to render onto screen. \n
     * Vertex provides information for each part of shape. \n
     * Quad has 4 vertices. \n
     * Triangle has 3 vertices.
     */
    struct ROBOT2D_EXPORT_API Vertex3D {
        /// Position of vertex without any transformation by default.
        mutable vec3f position;
        /// Texture coordinates. Values from 0.0 to 1.0.
        mutable vec2f texCoords;
        /// Color
        mutable Color color = Color::White;
    };

    using Vertex3DData = std::vector<Vertex3D>;
}