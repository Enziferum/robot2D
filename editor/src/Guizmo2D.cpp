/*********************************************************************
(c) Alex Raag 2024
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

#ifdef _MSC_VER
    #define _USE_MATH_DEFINES
    #include <cmath>
#endif



#include <robot2D/Core/Window.hpp>
#include <robot2D/Util/Logger.hpp>

#include <editor/Guizmo2D.hpp>
#include <editor/Macro.hpp>
#include <editor/EditorResourceManager.hpp>

namespace editor {
    namespace {
        robot2D::vec2f rotateSize = { 50.f, 50.f};
    }

    DebugDrawable::~DebugDrawable() =  default;



    robot2D::vec2f Guizmo2D::Manipulator::getSize() const {
        if(m_axis == Axis::BOTH)
            return {m_rect.width, m_rect.height};
        else
            return {m_sprite.getGlobalBounds().width, m_sprite.getGlobalBounds().height};
    }

    bool Guizmo2D::Manipulator::isPressed(robot2D::vec2f point) {
        robot2D::FloatRect rect;
        if(m_axis != Axis::BOTH)
            rect = m_sprite.getGlobalBounds();
        else
            rect = m_rect;
        m_active = rect.contains(point);
        if(m_active)
            m_offset = point - rect.topPoint();
        return m_active;
    }

    bool Guizmo2D::Manipulator::active() const { return m_active; }

    void Guizmo2D::Manipulator::setPosition(const robot2D::vec2f& position) {
        m_rect.lx = position.x;
        m_rect.ly = position.y;
        m_sprite.setPosition({position.x, position.y});
    }

    void Guizmo2D::Manipulator::setColor(robot2D::Color color) { m_color = color; }

    void Guizmo2D::Manipulator::moveX(float offset) {
        auto pos = m_sprite.getPosition();
        m_sprite.setPosition({pos.x + offset, pos.y});
        m_rect.lx += offset;
    }

    void Guizmo2D::Manipulator::moveY(float offset) {
        auto pos = m_sprite.getPosition();
        m_sprite.setPosition({pos.x, pos.y  + offset});
        m_rect.ly += offset;
    }

    void Guizmo2D::Manipulator::draw(robot2D::RenderTarget& target, [[maybe_unused]] robot2D::RenderStates states) const
    {
        if(m_axis != Axis::BOTH) {
            target.draw(m_sprite);
        }
        else {
            robot2D::Transform transform;
            transform.translate({m_rect.lx, m_rect.ly});
            transform.scale(m_rect.width, m_rect.height);
            states.color = m_color;
            states.transform *= transform;
            target.draw(states);
        }
    }


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

    void Guizmo2D::update() {}

    void Guizmo2D::setIsShow(bool flag) {
        m_isShown = flag;
    }

    void Guizmo2D::draw(robot2D::RenderTarget& target,
                        [[maybe_unused]] robot2D::RenderStates states) const {
        if(!m_isShown)
            return;

        if(m_operation != Operation::Rotate) {
            target.draw(m_xAxisManipulator);
            target.draw(m_yAxisManipulator);
            target.draw(m_XYAxisManipulator);
        }
        else {
            target.draw(m_rotateSprite);
        }

    }

    void Guizmo2D::processMouseMoved(robot2D::Event event) {
        if(!m_leftMousePressed) return;
        robot2D::vec2f moveVector{event.move.x, event.move.y};
        moveVector = m_camera -> convertPixelToCoords(moveVector);

        switch(m_operation) {
            case Operation::Move: {
                if(m_xAxisManipulator.active()) {
                    /// move transformAble by x
                    float diff = moveVector.x - m_manipulatorLastPos.x;
                    for(auto transform: m_manipulateds) {
                        auto pos = transform -> getPosition();
                        transform -> setPosition({pos.x + diff, pos.y});
                    }

                    m_manipulatorLastPos.x += diff;
                    m_xAxisManipulator.moveX(diff);
                    m_yAxisManipulator.moveX(diff);
                    m_XYAxisManipulator.moveX(diff);
                    /// update manipulator's position
                }
                else if(m_yAxisManipulator.active()) {
                    /// move transformAble by y
                    float diff = moveVector.y - m_manipulatorLastPos.y;
                    for(auto transform: m_manipulateds) {
                        auto pos = transform -> getPosition();
                        transform -> setPosition({pos.x, pos.y + diff});
                    }

                    m_manipulatorLastPos.y += diff;
                    m_xAxisManipulator.moveY(diff);
                    m_yAxisManipulator.moveY(diff);
                    m_XYAxisManipulator.moveY(diff);
                    /// update manipulator's position
                }
                else if(m_XYAxisManipulator.active()) {
                    robot2D::vec2f diff = moveVector - m_manipulatorLastPos;
                    for(auto transform: m_manipulateds) {
                        auto pos = transform -> getPosition();
                        transform -> setPosition(pos + diff);
                    }
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

                if (m_xAxisManipulator.active()) {
                    /// move transformAble by x
                    float diff = moveVector.x - m_manipulatorLastPos.x;
                    m_manipulatorLastPos.x += diff;
                    for(auto transform: m_manipulateds) {
                        auto size = transform -> getSize();
                        transform -> setSize( {size.x + diff, size.y} );
                    }
                    m_xAxisManipulator.moveX(diff);
                    m_yAxisManipulator.moveX(diff);
                    m_XYAxisManipulator.moveX(diff);
                    /// update manipulator's position
                } else if (m_yAxisManipulator.active()) {
                    /// move transformAble by y
                    float diff = moveVector.y - m_manipulatorLastPos.y;
                    m_manipulatorLastPos.y += diff;
                    for(auto transform: m_manipulateds) {
                        auto size = transform -> getSize();
                        transform -> setSize( {size.x, size.y + diff} );
                    }
                    m_xAxisManipulator.moveY(diff);
                    m_yAxisManipulator.moveY(diff);
                    m_XYAxisManipulator.moveY(diff);
                    /// update manipulator's position
                }
                else if(m_XYAxisManipulator.active()) {
                    robot2D::vec2f diff = moveVector - m_manipulatorLastPos;
                    for(auto transform: m_manipulateds) {
                        auto size = transform -> getSize();
                        transform -> setSize( size + diff );
                    }
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
            case Operation::Rotate: {
                robot2D::vec2f diff = moveVector - m_manipulatorLastPos;
                m_manipulatorLastPos += diff;
                float diffAngle = 1.f;
                if(diff.y < 0)
                    diffAngle = -1.f;

                for(auto& transform: m_manipulateds) {
                    auto lastRotate = transform -> getRotation();
                    transform -> setRotate(lastRotate + diffAngle);
                }
                break;
            }
        }
    }

    void Guizmo2D::processMousePressed(robot2D::Event event) {
        if(event.mouse.btn == robot2D::mouse2int(robot2D::Mouse::MouseLeft)) {
            robot2D::vec2f mousePoint{static_cast<float>(event.mouse.x), static_cast<float>(event.mouse.y)};
            mousePoint = m_camera -> convertPixelToCoords(mousePoint);
            auto bounds = m_rotateSprite.getGlobalBounds();
            if(m_xAxisManipulator.isPressed(mousePoint)
                || m_yAxisManipulator.isPressed(mousePoint)
                || m_XYAxisManipulator.isPressed(mousePoint) || bounds.contains(mousePoint)) {
                m_leftMousePressed = true;
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





    void Guizmo2D::setManipulated(TransformComponent* transformable) {
        m_manipulateds.clear();
        if(!transformable)
            return;
        m_manipulateds.emplace_back(transformable);

        auto m_manipulated = m_manipulateds[0];

        const auto& position = m_manipulated -> getPosition();
        auto size = transformable -> getSize();
        const auto& rotation = m_manipulated -> getRotate();

        if(m_operation != Operation::Rotate) {
            if(rotation == 0) {

                auto middle = robot2D::vec2f { position.x + size.x / 2,
                                               position.y + size.y / 2};

                auto xSize = m_xAxisManipulator.getSize();
                auto ySize = m_yAxisManipulator.getSize();
                auto xySize = m_XYAxisManipulator.getSize();

                constexpr float xyOffset = 3.f;

                m_xAxisManipulator.setPosition({middle.x, middle.y + xSize.y / 2.f});
                m_yAxisManipulator.setPosition({middle.x + ySize.x / 2.F, middle.y});
                m_XYAxisManipulator.setPosition(robot2D::vec2f{middle.x + xyOffset, middle.y - xySize.y - xyOffset});
            }
            else {
                auto bounds = transformable -> getGlobalBounds();

                auto xSize = m_xAxisManipulator.getSize();
                auto ySize = m_yAxisManipulator.getSize();
                auto xySize = m_XYAxisManipulator.getSize();

                constexpr float xyOffset = 3.f;

                auto middle = bounds.centerPoint();

                m_xAxisManipulator.setPosition( {middle.x, middle.y + xSize.y / 2.f} );
                m_yAxisManipulator.setPosition( {middle.x + ySize.x / 2.F, middle.y} );
                m_XYAxisManipulator.setPosition( {middle.x + xyOffset, middle.y - xySize.y - xyOffset} );
            }
        }
        else {
            auto bounds = transformable -> getGlobalBounds();
            auto middle = bounds.centerPoint();
            auto size = m_rotateSprite.getSize();
            m_rotateSprite.setPosition({middle.x, middle.y - size.y / 2.f});
        }


    }

    void Guizmo2D::setManipulated(std::vector<TransformComponent*>& transformables) {
        m_manipulateds = transformables;
        robot2D::vec2f minLeftPoint = transformables[0] -> getPosition();
        robot2D::vec2f maxRightPoint = transformables[0] -> getPosition();

        for(auto transform: transformables) {
            auto position = transform -> getPosition();
            if(position.x < minLeftPoint.x)
                minLeftPoint.x = position.x;
            if(position.y < minLeftPoint.y)
                minLeftPoint.y = position.y;
            if(position.x > maxRightPoint.x)
                minLeftPoint.x = position.x;
            if(position.y > maxRightPoint.y)
                maxRightPoint.y = position.y;
        }

        constexpr float xyOffset = 3.f;

        robot2D::vec2f distance = {maxRightPoint.x - minLeftPoint.x, maxRightPoint.y - minLeftPoint.y};

        auto middle = robot2D::vec2f{ minLeftPoint.x + distance.x / 2.f,
                                      minLeftPoint.y + distance.y / 2.f};
        auto xSize = m_xAxisManipulator.getSize();
        auto ySize = m_yAxisManipulator.getSize();
        auto xySize = m_XYAxisManipulator.getSize();

        m_xAxisManipulator.setPosition({middle.x, middle.y + xSize.y / 2.f});
        m_yAxisManipulator.setPosition({middle.x + ySize.x / 2.F, middle.y});
        m_XYAxisManipulator.setPosition({middle.x + xyOffset, middle.y - xySize.y - xyOffset});
    }

    void Guizmo2D::setOperationType(Guizmo2D::Operation type) {
        m_operation = type;
        switch(m_operation) {
            case Operation::Rotate: {
                m_rotateSprite.setSize(rotateSize);
                auto size = m_manipulatorRotateTexture.getSize();
                m_rotateSprite.setTexture(m_manipulatorRotateTexture,
                                          robot2D::IntRect{0, 0, (int)size.x, (int)size.y});
                return;
            }
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
        auto resourceManager = EditorResourceManager::getManager();
        if(!resourceManager -> hasTexture(EditorResourceID::Manipulator)) {
            if(!resourceManager -> loadFromFile(EditorResourceID::Manipulator, "gizmos.png")) {
                RB_EDITOR_ERROR("Gizmo2D: Can't load manipulator's texture");
                /// TODO(a.raag): throw error
            }
        }

        if(!resourceManager -> hasTexture(EditorResourceID::ManipulatorRotate)) {
            if(!resourceManager -> loadFromFile(EditorResourceID::ManipulatorRotate, "rotate.png")) {
                RB_EDITOR_ERROR("Gizmo2D: Can't load manipulator's rotate texture");
                /// TODO(a.raag): throw error
            }
        }



        m_manipulatorTexture = resourceManager -> getTexture(EditorResourceID::Manipulator);
        m_manipulatorRotateTexture = resourceManager -> getTexture(EditorResourceID::ManipulatorRotate);
        m_camera = camera;
        setOperationType(m_operation);
    }


}