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

#include <cmath>
#include <robot2D/Graphics/RenderTarget.hpp>
#include <editor/BoundingBox.hpp>

namespace editor {


    void BoundingBox::Wall::draw(robot2D::RenderTarget &target, robot2D::RenderStates states) const {

        robot2D::Transform transform;
        transform.translate(position);
        transform.scale(size);
        states.transform *= transform;
        states.color = color;
        target.draw(states);
    }

    BoundingBox::BoundingBox() {
        for(auto& wall: m_moveWalls)
            wall.color = m_borderColor;
    }


    void BoundingBox::setPosition(const robot2D::vec2f& position) {
        m_aabb.lx = position.x;
        m_aabb.ly = position.y;
    }

    void BoundingBox::setSize(const robot2D::vec2f& size) {
        m_aabb.width = size.x;
        m_aabb.height = size.y;
    }

    void BoundingBox::setAngle(float angle) {
        m_angle = static_cast<float>(fmod(angle, 360));
    }

    void BoundingBox::setBox(const robot2D::FloatRect& rect) {
        m_aabb = rect;
    }

    void BoundingBox::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {

        m_moveWalls[0].position = robot2D::vec2f(m_aabb.lx, m_aabb.ly - m_borderWidth);
        m_moveWalls[0].size = robot2D::vec2f(m_aabb.width + m_borderWidth, m_borderWidth);
        m_moveWalls[0].angle = m_angle;

        m_moveWalls[1].position = robot2D::vec2f(m_aabb.lx, m_aabb.ly);
        m_moveWalls[1].size = robot2D::vec2f(m_borderWidth, m_aabb.height);
        m_moveWalls[1].angle = m_angle;

        m_moveWalls[2].position = robot2D::vec2f(m_aabb.lx, m_aabb.ly + m_aabb.height);
        m_moveWalls[2].size = robot2D::vec2f(m_aabb.width, m_borderWidth);
        m_moveWalls[2].angle = m_angle;

        m_moveWalls[3].position = robot2D::vec2f(m_aabb.lx + m_aabb.width, m_aabb.ly);
        m_moveWalls[3].size = robot2D::vec2f(m_borderWidth, m_aabb.height + m_borderWidth);
        m_moveWalls[3].angle = m_angle;

        for(const auto& wall: m_moveWalls) {
            target.draw(wall);
        }

    }
} // namespace editor