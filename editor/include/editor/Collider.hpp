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

#include <array>
#include <robot2D/Graphics/Drawable.hpp>
#include <robot2D/Graphics/RenderTarget.hpp>

#include "Quad.hpp"
#include "EventBinder.hpp"
#include "EditorCamera.hpp"

namespace editor {
    struct Collider: public robot2D::Drawable {
        Collider();
        ~Collider() override = default;

        enum class State {
            Default,
            Selected,
            Moving
        } state = State::Default;

        void handleEvents(const robot2D::Event& event);

        void setCamera(IEditorCamera::Ptr camera) { m_camera = camera; }

        void setPosition(const robot2D::vec2f& position) {
            m_aabb.lx = position.x;
            m_aabb.ly = position.y;
        }

        robot2D::vec2f getPosition() const {
            return {m_aabb.lx, m_aabb.ly};
        }

        void setSize(const robot2D::vec2f& size) {
            m_aabb.width = size.x;
            m_aabb.height = size.y;
        }

        void setRect(const robot2D::vec2f& topLeft, const robot2D::vec2f& size) {
            m_aabb.lx = topLeft.x;
            m_aabb.ly = topLeft.y;
            m_aabb.width = size.x;
            m_aabb.height = size.y;
        }

        void setRect(const robot2D::FloatRect& rect) {
            m_aabb = rect;
        }

        const robot2D::FloatRect& getRect() const {
            return m_aabb;
        }

        bool contains(const robot2D::vec2f& point)  {
            return m_aabb.contains(point);
        }

        bool notZero() const {
            return m_aabb.width > 0 && m_aabb.height > 0;
        }

        bool intersects(const robot2D::FloatRect& rect) {
            return m_aabb.intersects(rect);
        }

        void setIsShown(bool flag) { m_show = flag; }

        void update(robot2D::vec2f mousePos, float dt) {
          //  if(m_needRecalculate)
                recalculateRect(mousePos, dt);
        }


        void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override {
            if(!m_show)
                return;

            std::array<robot2D::Transform, 4> quads;
            quads[0].translate(m_aabb.lx, m_aabb.ly);
            quads[0].scale(m_aabb.width, borderSize);

            quads[1].translate(m_aabb.lx, m_aabb.ly);
            quads[1].scale(borderSize, m_aabb.height);

            quads[2].translate(m_aabb.lx, m_aabb.ly + m_aabb.height);
            quads[2].scale(m_aabb.width, borderSize);

            quads[3].translate(m_aabb.lx + m_aabb.width, m_aabb.ly);
            quads[3].scale(borderSize, m_aabb.height);

            for(const auto& wall: moveWalls) {
//                states.transform = transform;
//                states.color = borderColor;
//                target.draw(states);
                target.draw(wall);
            }

            constexpr robot2D::vec2f qSize = {6, 6};

            moveQ[0].color = (m_selectedQuad == static_cast<int>(SelectQuad::Top) ? robot2D::Color::Yellow: robot2D::Color::White);
            moveQ[1].color = (m_selectedQuad == static_cast<int>(SelectQuad::Left) ? robot2D::Color::Yellow: robot2D::Color::White);
            moveQ[2].color = (m_selectedQuad == static_cast<int>(SelectQuad::Right) ? robot2D::Color::Yellow: robot2D::Color::White);
            moveQ[3].color = (m_selectedQuad == static_cast<int>(SelectQuad::Bottom) ? robot2D::Color::Yellow: robot2D::Color::White);

            for(const auto& q: moveQ)
                target.draw(q);
        }

        const float& getSize() const { return m_size; }
        void setSize(float size);

        bool isActive() const { return m_leftMousePressed && m_selectedQuad != -1 && m_pressedPoint != robot2D::vec2f{}; }
    public:
        robot2D::Color borderColor = robot2D::Color::White;
        float borderSize = 1.F;

    private:
        void onMousePressed(const robot2D::Event& event);
        void onMouseReleased(const robot2D::Event& event);
        void onMouseMoved(const robot2D::Event& event);
        void recalculateRect(robot2D::vec2f mousePos, float newSize);
    private:
        robot2D::FloatRect m_aabb{};
        EventBinder m_eventBinder;
        mutable std::array<Quad, 4> moveQ;
        mutable std::array<Quad, 4> moveWalls;
        bool m_leftMousePressed{false};
        bool m_show{false};
        float m_size = 1.f;
        float m_lastSize = 1.f;
        robot2D::vec2f m_pressedPoint{};
        enum class SelectQuad: int {
            None = -1,
            Top,
            Left,
            Right,
            Bottom
        };

        int m_selectedQuad = static_cast<int>(SelectQuad::None);
        IEditorCamera::Ptr m_camera{nullptr};
        bool m_needRecalculate{false};
    };

}