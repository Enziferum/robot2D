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

#include <editor/Macro.hpp>
#include <editor/Collider.hpp>
#include <robot2D/Core/Window.hpp>

namespace editor {

    namespace {
        constexpr float sizeStep = 0.1f;
        constexpr float minSize = 1.f;
        constexpr float pixelPerStep = 500.f;


        constexpr robot2D::vec2f qSize = {6, 6};
    }

    Collider::Collider() {
        m_eventBinder.bindEvent(robot2D::Event::EventType::MousePressed, BIND_CLASS_FN(onMousePressed));
        m_eventBinder.bindEvent(robot2D::Event::EventType::MouseReleased, BIND_CLASS_FN(onMouseReleased));
        m_eventBinder.bindEvent(robot2D::Event::EventType::MouseMoved, BIND_CLASS_FN(onMouseMoved));

        m_aabb.width = 178;
        m_aabb.height = 100;


        moveQ[0].size = qSize;
        moveQ[1].size = qSize;
        moveQ[2].size = qSize;
        moveQ[3].size = qSize;

        moveWalls[0].color = borderColor;
        moveWalls[1].color = borderColor;
        moveWalls[2].color = borderColor;
        moveWalls[3].color = borderColor;
    }

    void Collider::handleEvents(const robot2D::Event& event) {
        m_eventBinder.handleEvents(event);
    }

    void Collider::onMousePressed(const robot2D::Event& event) {
        if(event.type == robot2D::Event::MousePressed) {
            m_leftMousePressed = true;
            robot2D::vec2f pressedPoint{event.mouse.x, event.mouse.y};
            pressedPoint = m_camera -> convertPixelToCoords(pressedPoint);

            int index = 0;
            for(auto& quad: moveQ) {
                auto frame = quad.getFrame();
                if(frame.contains(pressedPoint)) {
                    auto botPoint = frame.botPoint();
                    m_pressedPoint = pressedPoint;
                    m_selectedQuad = index;
                    return;
                }
                ++index;
            }
        }
    }

    void Collider::onMouseReleased(const robot2D::Event& event) {
        if(event.type == robot2D::Event::MouseReleased) {
            m_leftMousePressed = false;
            m_pressedPoint = {};
            m_selectedQuad = -1;
        }
    }

    void Collider::onMouseMoved(const robot2D::Event& event) {

        if(event.type == robot2D::Event::MouseMoved) {

            robot2D::vec2f moveVector {
                event.move.x,
                event.move.y
            };

            moveVector = m_camera -> convertPixelToCoords(moveVector);

            if(!m_leftMousePressed)
                return;

           // auto frame = quad.getFrame();
            float diffSize = m_size;

            robot2D::vec2f diff = moveVector - m_pressedPoint;
            m_pressedPoint = moveVector;

            if(m_selectedQuad == 0) {
                if(diff.y < 0 ) {
                    diffSize += sizeStep;
                }
                else if(diff.y > 0 && (diffSize - sizeStep >= minSize)) {
                    diffSize -= sizeStep;
                }
            }

            if(m_selectedQuad == 1) {
                if(diff.x < 0 ) {
                    diffSize += sizeStep;
                }
                else if(diff.x > 0 && (diffSize - sizeStep >= minSize)) {
                    diffSize -= sizeStep;
                }
            }

            if(m_selectedQuad == 2) {
                if(diff.x > 0) {
                    diffSize += sizeStep;
                }
                else if(diff.x < 0 && (diffSize - sizeStep >= minSize)) {
                    diffSize -= sizeStep;
                }
            }

            if(m_selectedQuad == 3) {
                if(diff.y > 0) {
                    diffSize += sizeStep;
                }
                else if(diff.y < 0 && (diffSize - sizeStep >= minSize)) {
                    diffSize -= sizeStep;
                }
            }

            if(diffSize == 0.f)
                return;

            m_needRecalculate = true;
            m_size = diffSize;
        }
    }

    void Collider::setSize(float size) {
        m_needRecalculate = true;
        m_size = size;
    }

    void Collider::recalculateRect(robot2D::vec2f mousePos, float dt) {
        float diffSize = m_size;

        mousePos = m_camera -> convertPixelToCoords(mousePos);
        robot2D::vec2f diff = mousePos - m_pressedPoint;
        m_pressedPoint = mousePos;

        if(m_selectedQuad == static_cast<int>(SelectQuad::Top)) {
            if(diff.y < 0 ) {
                diffSize += sizeStep;
            }
            else if(diff.y > 0 && (diffSize - sizeStep >= minSize)) {
                diffSize -= sizeStep;
            }
        }

        if(m_selectedQuad == static_cast<int>(SelectQuad::Left)) {
            if(diff.x < 0 ) {
                diffSize += sizeStep;
            }
            else if(diff.x > 0 && (diffSize - sizeStep >= minSize)) {
                diffSize -= sizeStep;
            }
        }

        if(m_selectedQuad == static_cast<int>(SelectQuad::Right)) {
            if(diff.x > 0) {
                diffSize += sizeStep;
            }
            else if(diff.x < 0 && (diffSize - sizeStep >= minSize)) {
                diffSize -= sizeStep;
            }
        }

        if(m_selectedQuad == static_cast<int>(SelectQuad::Bottom)) {
            if(diff.y > 0) {
                diffSize += sizeStep;
            }
            else if(diff.y < 0 && (diffSize - sizeStep >= minSize)) {
                diffSize -= sizeStep;
            }
        }

        m_size = diffSize;


        diffSize = m_lastSize - m_size;
        if(diffSize == 0.f) {}
        else if(diffSize > 0) {
            //   m_aabb.lx += pixelPerStep * dt * cos(45);
            //  m_aabb.ly -= pixelPerStep * dt * sin(45);
            m_aabb.width -= pixelPerStep * 2 * dt;
            m_aabb.height -= pixelPerStep * 2 * dt;
        }
        else if(diffSize < 0) {
            diffSize = std::abs(diffSize);
            //  m_aabb.lx -= pixelPerStep * dt * cos(45);
            //  m_aabb.ly += pixelPerStep * dt * sin(45);
            m_aabb.width += pixelPerStep * 2 * dt;
            m_aabb.height += pixelPerStep * 2 * dt;
        }
        m_needRecalculate = false;
        m_lastSize = m_size;

        moveQ[0].position = {
                ( m_aabb.lx + m_aabb.width / 2.F ) - qSize.x / 2.F,
                m_aabb.ly - qSize.y / 2.F
        };

        moveQ[1].position = {
                m_aabb.lx - qSize.x / 2.F,
                ( m_aabb.ly + m_aabb.height / 2.F ) - qSize.y / 2.F
        };


        moveQ[2].position = {
                ( m_aabb.lx + m_aabb.width ) - qSize.x / 2.F,
                ( m_aabb.ly + m_aabb.height / 2.f ) - qSize.y / 2.F
        };


        moveQ[3].position = {
                ( m_aabb.lx + m_aabb.width / 2.f ) - qSize.x / 2.F,
                m_aabb.ly + m_aabb.height - qSize.y / 2.F
        };


        moveWalls[0].position = robot2D::vec2f(m_aabb.lx, m_aabb.ly);
        moveWalls[0].size = robot2D::vec2f(m_aabb.width, borderSize);

        moveWalls[1].position = robot2D::vec2f(m_aabb.lx, m_aabb.ly);
        moveWalls[1].size = robot2D::vec2f(borderSize, m_aabb.height);

        moveWalls[2].position = robot2D::vec2f(m_aabb.lx, m_aabb.ly + m_aabb.height);
        moveWalls[2].size = robot2D::vec2f(m_aabb.width, borderSize);

        moveWalls[3].position = robot2D::vec2f(m_aabb.lx + m_aabb.width, m_aabb.ly);
        moveWalls[3].size = robot2D::vec2f(borderSize, m_aabb.height);
    }

}