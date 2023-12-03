/*********************************************************************
(c) Alex Raag 2023
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

#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Graphics/View.hpp>
#include <robot2D/Graphics/VertexArray.hpp>

namespace editor {

    struct GridVertex {
        robot2D::vec2f pos;
        robot2D::vec2f texCoord;

        GridVertex():pos{}, texCoord{}{}
        GridVertex(robot2D::vec2f p, robot2D::vec2f t):pos{p}, texCoord{t}{};
    };

    class SceneGrid {
    public:
        SceneGrid() = default;
        ~SceneGrid() = default;

        void setGridElementSize(const std::size_t& size);
        void setGridColors(const robot2D::Color& one, const robot2D::Color& two);
        void setZoomFactor();


        bool setup();
        void render(robot2D::vec2i size) const;
    private:
        robot2D::ShaderHandler m_shader{};
        robot2D::Color m_one{};
        robot2D::Color m_two{};

        robot2D::VertexArray::Ptr m_vertexArray{nullptr};
        mutable std::array<GridVertex, 4> m_vertices;
    };
} // namespace editor