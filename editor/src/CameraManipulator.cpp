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

#include <robot2D/Core/Window.hpp>
#include <editor/Macro.hpp>
#include <editor/CameraManipulator.hpp>
#include <editor/EditorResourceManager.hpp>
#include <editor/Messages.hpp>

namespace editor {

    namespace {
        constexpr float sizeStep = 0.1f;
        constexpr float minSize = 1.f;
        constexpr float pixelPerStep = 500.f;
        constexpr robot2D::vec2f minRectSize = {894, 489};
        constexpr robot2D::vec2f movieButtonSize = {20.f, 20.f};
        constexpr robot2D::vec2f qSize = {6, 6};
    }

    CameraManipulator::CameraManipulator(robot2D::MessageBus& messageBus): m_messageBus{messageBus}
    {
        m_eventBinder.bindEvent(robot2D::Event::EventType::MousePressed, BIND_CLASS_FN(onMousePressed));
        m_eventBinder.bindEvent(robot2D::Event::EventType::MouseReleased, BIND_CLASS_FN(onMouseReleased));
        m_eventBinder.bindEvent(robot2D::Event::EventType::MouseMoved, BIND_CLASS_FN(onMouseMoved));

        auto resourceManager = EditorResourceManager::getManager();
        if(!resourceManager -> hasTexture(EditorResourceID::Movie)) {
            if(!resourceManager -> loadFromFile(EditorResourceID::Movie, "movie.png")) {
                ///
            }
        }

        auto textureSize = resourceManager -> getTexture(EditorResourceID::Movie).getSize();
        m_movieSprite.setTexture(resourceManager -> getTexture(EditorResourceID::Movie),
                {0, 0,
                 static_cast<int>(textureSize.x),
                 static_cast<int>(textureSize.y)}
        );
        m_movieSprite.setSize(movieButtonSize);

        m_aabb.width = minRectSize.x;
        m_aabb.height = minRectSize.y;

        m_cameraView.reset({0, 0, minRectSize.x, minRectSize.y});

        moveQ[0].size = qSize;
        moveQ[1].size = qSize;
        moveQ[2].size = qSize;
        moveQ[3].size = qSize;

        moveWalls[0].color = borderColor;
        moveWalls[1].color = borderColor;
        moveWalls[2].color = borderColor;
        moveWalls[3].color = borderColor;
    }

    void CameraManipulator::handleEvents(const robot2D::Event& event) {
        m_eventBinder.handleEvents(event);
    }


    void CameraManipulator::onMousePressed(const robot2D::Event& event) {
        if(event.type == robot2D::Event::MousePressed) {
            m_leftMousePressed = true;
            robot2D::vec2f pressedPoint{static_cast<float>(event.mouse.x), static_cast<float>(event.mouse.y)};
            pressedPoint = m_camera -> convertPixelToCoords(pressedPoint);


            if(m_movieSprite.getGlobalBounds().contains(pressedPoint)) {
               // m_buttonCallback(m_manipulatedEntity);
            }

            int index = 0;
            for(auto& quad: moveQ) {
                auto frame = quad.getFrame();
                if(frame.contains(pressedPoint)) {
                    m_pressedPoint = pressedPoint;
                    m_selectedQuad = index;
                    return;
                }
                ++index;
            }

        }
    }

    void CameraManipulator::onMouseReleased(const robot2D::Event& event) {
        if(event.type == robot2D::Event::MouseReleased) {
            m_leftMousePressed = false;
            m_pressedPoint = {};
            m_selectedQuad = -1;
        }
    }

    void CameraManipulator::onMouseMoved(const robot2D::Event& event) {
        if(event.type != robot2D::Event::MouseMoved || !m_leftMousePressed)
            return;

        robot2D::vec2f moveVector {
            event.move.x,
            event.move.y
        };

        moveVector = m_camera -> convertPixelToCoords(moveVector);
        float diffSize = m_size;

        robot2D::vec2f diff = moveVector - m_pressedPoint;
        m_pressedPoint = moveVector;

        bool inscrease = true;

        if(m_selectedQuad == 0) {
            if(diff.y < 0 ) {
                diffSize += sizeStep;
                inscrease = true;
            }
            else if(diff.y > 0) {
                diffSize -= sizeStep;
                inscrease = false;
            }
        }

        if(m_selectedQuad == 1) {
            if(diff.x < 0 ) {
                diffSize += sizeStep;
                inscrease = true;
            }
            else if(diff.x > 0) {
                diffSize -= sizeStep;
                inscrease = false;
            }
        }

        if(m_selectedQuad == 2) {
            if(diff.x > 0) {
                diffSize += sizeStep;
                inscrease = true;
            }
            else if(diff.x < 0) {
                diffSize -= sizeStep;
                inscrease = false;
            }
        }

        if(m_selectedQuad == 3) {
            if(diff.y > 0) {
                diffSize += sizeStep;
                inscrease = true;
            }
            else if(diff.y < 0) {
                diffSize -= sizeStep;
                inscrease = false;
            }
        }

        if(diffSize == 0.f)
            return;

        m_needRecalculate = true;
        m_size = diffSize;

        if(m_selectedQuad != static_cast<int>(SelectQuad::None)) {
            float factor = inscrease ? 1.01f : 0.99f;
            m_cameraView.zoom(factor);
        }

    }


    void CameraManipulator::setSize(float size) {
        m_needRecalculate = true;
        m_size = size;
    }

    void CameraManipulator::recalculateRect(robot2D::vec2f mousePos, float dt) {
        float diffSize = m_size;
        robot2D::vec2f aspectCoof{16/9, 1};


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


        m_aabb = m_cameraView.getRectangle();


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

    void CameraManipulator::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
        if(!m_manipulatedEntity)
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


        for(const auto& wall: moveWalls)
            target.draw(wall);

        target.draw(m_movieSprite);

        if(!m_showDots)
            return;
        
        moveQ[0].color = (m_selectedQuad == static_cast<int>(SelectQuad::Top) ? robot2D::Color::Yellow: robot2D::Color::White);
        moveQ[1].color = (m_selectedQuad == static_cast<int>(SelectQuad::Left) ? robot2D::Color::Yellow: robot2D::Color::White);
        moveQ[2].color = (m_selectedQuad == static_cast<int>(SelectQuad::Right) ? robot2D::Color::Yellow: robot2D::Color::White);
        moveQ[3].color = (m_selectedQuad == static_cast<int>(SelectQuad::Bottom) ? robot2D::Color::Yellow: robot2D::Color::White);

        for(const auto& q: moveQ)
            target.draw(q);
    }

}