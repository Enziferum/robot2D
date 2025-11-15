/*********************************************************************
(c) Alex Raag 2025
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

#include <robot2D/Graphics/Drawable.hpp>
#include <robot2D/Graphics/Transformable.hpp>

namespace editor {

    struct BoundingBox: robot2D::Drawable {
        struct Wall: robot2D::Drawable {
            void draw(robot2D::RenderTarget& target,
                      robot2D::RenderStates states) const;

            robot2D::Color color;
            robot2D::vec2f position;
            robot2D::vec2f size;
            float angle = 0.f;
        };

        BoundingBox();
        BoundingBox(const BoundingBox& other) = default;
        BoundingBox& operator=(const BoundingBox& other) = default;
        BoundingBox(BoundingBox&& other) = default;
        BoundingBox& operator=(BoundingBox&& other) = default;
        ~BoundingBox() override = default;

        void setPosition(const robot2D::vec2f& position);
        void setSize(const robot2D::vec2f& size);
        void setAngle(float angle);
        void setBox(const robot2D::FloatRect& rect);

        void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override;
    private:
        robot2D::Color m_borderColor = robot2D::Color::Cyan;
        mutable std::array<Wall, 4> m_moveWalls;
        robot2D::FloatRect m_aabb;
        float m_borderWidth = 1.f;
        float m_angle;
    };

} // namespace editor