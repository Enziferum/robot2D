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
#include <robot2D/Util/Logger.hpp>
#include <editor/Guizmo2D.hpp>
#include <editor/Macro.hpp>

namespace editor {
    DebugDrawable::~DebugDrawable() =  default;

    Guizmo2D::Guizmo2D():
            m_xAxisManipulator{Manipulator::Axis::X, robot2D::Color::Red},
            m_yAxisManipulator{Manipulator::Axis::Y, robot2D::Color::Green},
            m_XYAxisManipulator{Manipulator::Axis::BOTH, robot2D::Color::Blue}
    {
        m_eventBinder.bindEvent(robot2D::Event::MousePressed, BIND_CLASS_FN(processMousePressed));
        m_eventBinder.bindEvent(robot2D::Event::MouseReleased, BIND_CLASS_FN(processMouseRelease));
        m_eventBinder.bindEvent(robot2D::Event::MouseMoved, BIND_CLASS_FN(processMouseMoved));
    }

    void Guizmo2D::handleEvents(const robot2D::Event& event) {
        if(!m_isShown)
            return;
        m_eventBinder.handleEvents(event);
    }

    void Guizmo2D::setIsShow(bool flag) {
        m_isShown = flag;
    }

    void Guizmo2D::draw(robot2D::RenderTarget& target,
                        [[maybe_unused]] robot2D::RenderStates states) const {
        if(!m_isShown)
            return;

        target.draw(m_xAxisManipulator);
        target.draw(m_yAxisManipulator);
        target.draw(m_XYAxisManipulator);
    }

    void Guizmo2D::processMouseMoved(robot2D::Event event) {
        if(!m_leftMousePressed) return;
        robot2D::vec2f moveVector{event.move.x, event.move.y};
        moveVector = m_camera -> convertPixelToCoords(moveVector);

        switch(m_operation) {
            case Operation::Move: {
                auto pos = m_manipulated -> getPosition();
                if(m_xAxisManipulator.active()) {
                    /// move transformAble by x
                    float diff = moveVector.x - m_manipulatorLastPos.x;
                    m_manipulated -> setPosition({pos.x + diff, pos.y});
                    m_manipulatorLastPos.x += diff;
                    m_xAxisManipulator.moveX(diff);
                    m_yAxisManipulator.moveX(diff);
                    m_XYAxisManipulator.moveX(diff);
                    /// update manipulator's position
                }
                else if(m_yAxisManipulator.active()) {
                    /// move transformAble by y
                    float diff = moveVector.y - m_manipulatorLastPos.y;
                    m_manipulated -> setPosition({pos.x, pos.y + diff});
                    m_manipulatorLastPos.y += diff;
                    m_xAxisManipulator.moveY(diff);
                    m_yAxisManipulator.moveY(diff);
                    m_XYAxisManipulator.moveY(diff);
                    /// update manipulator's position
                }
                else if(m_XYAxisManipulator.active()) {
                    robot2D::vec2f diff = moveVector - m_manipulatorLastPos;
                    m_manipulated -> setPosition(pos + diff);
                    m_manipulatorLastPos += diff;
                    m_xAxisManipulator.moveX(diff.x);
                    m_xAxisManipulator.moveY(diff.y);
                    m_yAxisManipulator.moveX(diff.x);
                    m_yAxisManipulator.moveY(diff.y);
                    m_XYAxisManipulator.moveX(diff.x);
                    m_XYAxisManipulator.moveY(diff.y);
                }

                break;
            }
            case Operation::Scale: {
                auto size = m_manipulated -> getScale();
                if (m_xAxisManipulator.active()) {
                    /// move transformAble by x
                    float diff = moveVector.x - m_manipulatorLastPos.x;
                    m_manipulatorLastPos.x += diff;
                    m_manipulated -> setScale( {size.x + diff, size.y} );
                    m_xAxisManipulator.moveX(diff);
                    m_yAxisManipulator.moveX(diff);
                    m_XYAxisManipulator.moveX(diff);
                    /// update manipulator's position
                } else if (m_yAxisManipulator.active()) {
                    /// move transformAble by y
                    float diff = moveVector.y - m_manipulatorLastPos.y;
                    m_manipulatorLastPos.y += diff;
                    m_manipulated -> setScale( {size.x , size.y + diff} );
                    m_xAxisManipulator.moveY(diff);
                    m_yAxisManipulator.moveY(diff);
                    m_XYAxisManipulator.moveY(diff);
                    /// update manipulator's position
                }
                else if(m_XYAxisManipulator.active()) {
                    robot2D::vec2f diff = moveVector - m_manipulatorLastPos;
                    m_manipulated -> setScale(size + diff);
                    m_manipulatorLastPos += diff;
                    m_xAxisManipulator.moveX(diff.x);
                    m_xAxisManipulator.moveY(diff.y);
                    m_yAxisManipulator.moveX(diff.x);
                    m_yAxisManipulator.moveY(diff.y);
                    m_XYAxisManipulator.moveX(diff.x);
                    m_XYAxisManipulator.moveY(diff.y);
                }
                break;
            }
            case Operation::Rotate:
                break;
        }
    }

    void Guizmo2D::processMousePressed(robot2D::Event event) {
        if(event.mouse.btn == robot2D::mouse2int(robot2D::Mouse::MouseLeft)) {
            robot2D::vec2f mousePoint{event.mouse.x, event.mouse.y};
            mousePoint = m_camera -> convertPixelToCoords(mousePoint);
            m_leftMousePressed = true;
            if(m_xAxisManipulator.isPressed(mousePoint)
                || m_yAxisManipulator.isPressed(mousePoint)
                || m_XYAxisManipulator.isPressed(mousePoint)) {
                m_manipulatorLastPos = mousePoint;
                m_isCollide = true;
            }
            else
                m_isCollide = false;
        }
    }

    void Guizmo2D::processMouseRelease(robot2D::Event event) {
        if(event.mouse.btn == robot2D::mouse2int(robot2D::Mouse::MouseLeft)) {
            m_leftMousePressed = false;
            m_isCollide = false;
        }
    }

    void Guizmo2D::update() {}

    void Guizmo2D::setManipulated(robot2D::Transformable* transformable) {
        m_manipulated = transformable;
        if(!m_manipulated)
            return;
        const auto& position = m_manipulated -> getPosition();
        auto size = transformable -> getScale();

        auto middle = robot2D::vec2f { position.x + size.x / 2,
                                       position.y + size.y / 2};

        auto xSize = m_xAxisManipulator.getSize();
        auto ySize = m_yAxisManipulator.getSize();
        auto xySize = m_XYAxisManipulator.getSize();

        constexpr float xyOffset = 3.f;

        m_xAxisManipulator.setPosition({middle.x, middle.y + xSize.y / 2.f});
        m_yAxisManipulator.setPosition({middle.x + ySize.x / 2.F, middle.y});
        m_XYAxisManipulator.setPosition({middle.x + xyOffset, middle.y - xySize.y - xyOffset});
    }

    void Guizmo2D::setOperationType(Guizmo2D::Operation type) {
        m_operation = type;
        switch(m_operation) {
            case Operation::Rotate:
                return;
            case Operation::Move: {
                auto& xSprite = m_xAxisManipulator.m_sprite;
                xSprite.setRotate(270);
                xSprite.setColor(robot2D::Color::Red);
                xSprite.setTexture(m_manipulatorTexture, m_textureRects[TextureType::Move]);

                auto& ySprite = m_yAxisManipulator.m_sprite;
                ySprite.setRotate(180);
                ySprite.setColor(robot2D::Color::Green);
                ySprite.setTexture(m_manipulatorTexture, m_textureRects[TextureType::Move]);
                m_XYAxisManipulator.setColor(robot2D::Color{0.f, 0.f, 255.f, 100.f});
                break;
            }
            case Operation::Scale: {
                auto& xSprite = m_xAxisManipulator.m_sprite;
                xSprite.setRotate(270);
                xSprite.setColor(robot2D::Color::Red);
                xSprite.setTexture(m_manipulatorTexture, m_textureRects[TextureType::Scale]);

                auto& ySprite = m_yAxisManipulator.m_sprite;
                ySprite.setRotate(180);
                ySprite.setColor(robot2D::Color::Green);
                ySprite.setTexture(m_manipulatorTexture, m_textureRects[TextureType::Scale]);
                break;
            }
        }
    }

    void Guizmo2D::setCamera(IEditorCamera::Ptr camera) {
        m_manipulatorTexture.loadFromFile("res/icons/gizmos.png");
        m_camera = camera;
        setOperationType(m_operation);
    }

}