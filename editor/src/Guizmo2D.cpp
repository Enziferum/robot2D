#include <robot2D/Core/Window.hpp>
#include <robot2D/Util/Logger.hpp>
#include <editor/Guizmo2D.hpp>
#include <editor/Macro.hpp>

namespace editor {

    Guizmo2D::Guizmo2D():
            m_xAxisManipulator{Manipulator::Axis::X, robot2D::Color::Red},
            m_yAxisManipulator{Manipulator::Axis::Y, robot2D::Color::Green}
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

    void Guizmo2D::draw(robot2D::RenderTarget& target, [[maybe_unused]] robot2D::RenderStates states) const {
        if(!m_isShown)
            return;

        target.draw(m_xAxisManipulator);
        target.draw(m_yAxisManipulator);
    }

    void Guizmo2D::processMouseMoved(robot2D::Event event) {
        if(!m_leftMousePressed) return;
        robot2D::vec2f moveVector{event.move.x, event.move.y};
        moveVector = m_camera -> convertPixelToCoords(moveVector);

        switch(m_operation) {
            case Operation::Move: {
                auto pos = m_manipulated -> getPosition();
                auto size = m_manipulated -> getScale();
                if(m_xAxisManipulator.active()) {
                    /// move transformAble by x
                    moveVector.x -= size.x / 2;
                    m_manipulated -> setPosition({moveVector.x, pos.y});
                    m_xAxisManipulator.moveX(moveVector.x);
                    m_yAxisManipulator.moveX(moveVector.x);
                    /// update manipulator's position
                }
                else if(m_yAxisManipulator.active()) {
                    /// move transformAble by y
                    moveVector.y -= size.y / 2;
                    m_manipulated -> setPosition({pos.x, moveVector.y});
                    m_xAxisManipulator.moveY(moveVector.y);
                    m_yAxisManipulator.moveY(moveVector.y);
                    /// update manipulator's position
                }

                break;
            }
            case Operation::Scale:
                break;
            case Operation::Rotate:
                break;
        }
    }

    void Guizmo2D::processMousePressed(robot2D::Event event) {
        if(event.mouse.btn == robot2D::mouse2int(robot2D::Mouse::MouseLeft)) {
            robot2D::vec2f mousePoint{event.mouse.x, event.mouse.y};
            mousePoint = m_camera -> convertPixelToCoords(mousePoint);
            m_leftMousePressed = true;
            if(m_xAxisManipulator.isPressed(mousePoint)) {}

            if(m_yAxisManipulator.isPressed(mousePoint)) {}
        }
    }

    void Guizmo2D::processMouseRelease(robot2D::Event event) {
        if(event.mouse.btn == robot2D::mouse2int(robot2D::Mouse::MouseLeft)) {
            m_leftMousePressed = false;
        }
    }

    void Guizmo2D::update() {}

    void Guizmo2D::setManipulated(robot2D::Transformable* transformable) {
        m_manipulated = transformable;
        const auto& position = m_manipulated -> getPosition();
        auto size = transformable -> getScale();

        auto middle = robot2D::vec2f { position.x + size.x / 2,
                                       position.y + size.y / 2};

        m_xAxisManipulator.setPosition(middle);
        m_yAxisManipulator.setPosition(middle);
        /// get middle ///

    }
}