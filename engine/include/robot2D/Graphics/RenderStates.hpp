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

#include <robot2D/Config.hpp>
#include "Transform.hpp"
#include "Color.hpp"
#include "Shader.hpp"

namespace robot2D {
    class Texture;

    /// Render Shape Format
    enum class PrimitiveRenderType {
        Point,
        Lines,
        Triangles,
        Quads
    };

    /// \brief Graphics API must know how to render your Entity / Buffer.
    struct ROBOT2D_EXPORT_API RenderInfo {
        /// How to render vertices
        PrimitiveRenderType renderType = PrimitiveRenderType::Triangles;

        /// Value of specified indices for each Vertex.
        uint32_t indexCount = 0;
    };

    /**
     * \brief Contains specific options / values how to render Entity.
     */
    struct ROBOT2D_EXPORT_API RenderStates {
        RenderStates();
        ~RenderStates() = default;

        /// Entity Texture
        const Texture* texture;
        /// Allow to render with special Shader
        ShaderHandler* shader;
        /// Custom color
        Color color;
        /// Transformations. Check Transform.
        Transform transform;

        /// Graphics API render information. Check RenderInfo.
        RenderInfo* renderInfo;

        static const RenderStates Default;
    };
}