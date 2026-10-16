/*********************************************************************
   (c) Alex Raag 2026
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
#include <cmath>
#include <algorithm>
#include <robot2D/Util/Logger.hpp>

#include <editor/Guizmo2D.hpp>
#include <editor/Macro.hpp>


namespace editor {
    namespace {
        robot2D::vec2f rotateSize = { 50.f, 50.f};
        const robot2D::Color xColor{230.f, 87.f, 99.f};
        const robot2D::Color yColor{98.f, 198.f, 128.f};
        const robot2D::Color planeColor{91.f, 156.f, 227.f, 130.f};
        const robot2D::Color hoverColor{255.f, 220.f, 132.f};
        const robot2D::Color activeColor{255.f, 190.f, 70.f};

        void drawRing(robot2D::RenderTarget& target, robot2D::RenderStates states,
            const robot2D::FloatRect& bounds, robot2D::vec2f units, float thickness,
            const robot2D::Color& color) {
            constexpr int segments = 64;
            const auto center = bounds.centerPoint();
            const robot2D::vec2f outer{bounds.width * 0.5f, bounds.height * 0.5f};
            const robot2D::vec2f inner{std::max(0.f, outer.x - thickness * units.x),
                                       std::max(0.f, outer.y - thickness * units.y)};
            states.texture = nullptr;
            states.color = color;
            robot2D::VertexData vertices(4);
            for(int index = 0; index < segments; ++index) {
                const float first = static_cast<float>(index) * 2.f * static_cast<float>(M_PI) / segments;
                const float second = static_cast<float>(index + 1) * 2.f * static_cast<float>(M_PI) / segments;
                vertices[0].position = center + robot2D::vec2f{std::cos(first) * outer.x, std::sin(first) * outer.y};
                vertices[1].position = center + robot2D::vec2f{std::cos(second) * outer.x, std::sin(second) * outer.y};
                vertices[2].position = center + robot2D::vec2f{std::cos(second) * inner.x, std::sin(second) * inner.y};
                vertices[3].position = center + robot2D::vec2f{std::cos(first) * inner.x, std::sin(first) * inner.y};
                target.draw(vertices, states);
            }
        }
    }

    DebugDrawable::~DebugDrawable() =  default;



    robot2D::vec2f Guizmo2D::Manipulator::getSize() const {
        if(m_axis == Axis::BOTH)
            return {m_rect.width, m_rect.height};
        else
            return {m_sprite.getGlobalBounds().width, m_sprite.getGlobalBounds().height};
    }

    bool Guizmo2D::Manipulator::isPressed(robot2D::vec2f point) {
        m_active = contains(point);
        if(m_active)
            m_offset = point - m_rect.topPoint();
        return m_active;
    }

    bool Guizmo2D::Manipulator::contains(robot2D::vec2f point) const {
        return m_axis == Axis::BOTH ? m_rect.contains(point) : m_sprite.getGlobalBounds().contains(point);
    }

    void Guizmo2D::Manipulator::setSize(robot2D::vec2f size) {
        m_rect.width = size.x;
        m_rect.height = size.y;
        m_sprite.setSize(size);
    }

    bool Guizmo2D::Manipulator::active() const {
        return m_active;
    }

    void Guizmo2D::Manipulator::setPosition(const robot2D::vec2f& position) {
        m_rect.lx = position.x;
        m_rect.ly = position.y;
        m_sprite.setPosition({position.x, position.y});
    }

    void Guizmo2D::Manipulator::setColor(robot2D::Color color) {
        m_color = color;
        m_sprite.setColor(color);
    }

    void Guizmo2D::Manipulator::moveX(float offset) {
        auto pos = m_sprite.getPosition();
        m_sprite.setPosition({pos.x + offset, pos.y});
        m_rect.lx += offset;
    }

    void Guizmo2D::Manipulator::moveY(float offset) {
        auto pos = m_sprite.getPosition();
        m_sprite.setPosition({pos.x, pos.y + offset});
        m_rect.ly += offset;
    }

    void Guizmo2D::Manipulator::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
        const auto bounds = m_axis == Axis::BOTH ? m_rect : m_sprite.getGlobalBounds();
        states.texture = nullptr;
        states.color = m_color;
        robot2D::VertexData vertices(4);
        auto quad = [&](robot2D::vec2f first, robot2D::vec2f second,
            robot2D::vec2f third, robot2D::vec2f fourth) {
            vertices[0].position = first;
            vertices[1].position = second;
            vertices[2].position = third;
            vertices[3].position = fourth;
            target.draw(vertices, states);
        };
        auto rect = [&](float x, float y, float width, float height) {
            quad({x, y}, {x + width, y}, {x + width, y + height}, {x, y + height});
        };
        if(m_axis == Axis::BOTH) {
            rect(bounds.lx, bounds.ly, bounds.width, bounds.height);
            states.color.alpha = 230.f;
            const float borderX = bounds.width / 20.f;
            const float borderY = bounds.height / 20.f;
            rect(bounds.lx, bounds.ly, bounds.width, borderY);
            rect(bounds.lx, bounds.ly + bounds.height - borderY, bounds.width, borderY);
            rect(bounds.lx, bounds.ly, borderX, bounds.height);
            rect(bounds.lx + bounds.width - borderX, bounds.ly, borderX, bounds.height);
        }
        else if(m_axis == Axis::X) {
            const float center = bounds.ly + bounds.height * 0.5f;
            const float head = bounds.width * (m_scaleHandle ? 0.16f : 0.2f);
            rect(bounds.lx, center - bounds.height * 0.1f, bounds.width - head, bounds.height * 0.2f);
            const float end = bounds.lx + bounds.width;
            if(m_scaleHandle)
                rect(end - head, center - bounds.height * 0.4f, head, bounds.height * 0.8f);
            else
                quad({end - head, bounds.ly}, {end, center},
                    {end - head, bounds.ly + bounds.height}, {end - head, bounds.ly + bounds.height});
        }
        else {
            const float center = bounds.lx + bounds.width * 0.5f;
            const float head = bounds.height * (m_scaleHandle ? 0.16f : 0.2f);
            rect(center - bounds.width * 0.1f, bounds.ly + head, bounds.width * 0.2f, bounds.height - head);
            if(m_scaleHandle)
                rect(center - bounds.width * 0.4f, bounds.ly, bounds.width * 0.8f, head);
            else
                quad({bounds.lx, bounds.ly + head}, {center, bounds.ly},
                    {bounds.lx + bounds.width, bounds.ly + head}, {bounds.lx + bounds.width, bounds.ly + head});
        }
    }
    Guizmo2D::Guizmo2D() :
        m_xAxisManipulator{Manipulator::Axis::X, robot2D::Color::Red},
        m_yAxisManipulator{Manipulator::Axis::Y, robot2D::Color::Green},
        m_XYAxisManipulator{Manipulator::Axis::BOTH, robot2D::Color::Blue} {
        m_eventBinder.bindEvent(robot2D::Event::MousePressed, BIND_CLASS_FN(processMousePressed));
        m_eventBinder.bindEvent(robot2D::Event::MouseReleased, BIND_CLASS_FN(processMouseRelease));
        m_eventBinder.bindEvent(robot2D::Event::MouseMoved, BIND_CLASS_FN(processMouseMoved));
    }

    void Guizmo2D::handleEvents(const robot2D::Event& event) {
        if(event.type == robot2D::Event::LostFocus || event.type == robot2D::Event::MouseReleased) {
            if(event.type == robot2D::Event::LostFocus
                || event.mouse.btn == robot2D::mouse2int(robot2D::Mouse::MouseLeft))
                cancelDrag();
            return;
        }
        if(!m_isShown || !m_camera || m_manipulateds.empty())
            return;
        m_eventBinder.handleEvents(event);
    }

    void Guizmo2D::update() {
        updateColors();
    }

    void Guizmo2D::updateHover(robot2D::vec2f mousePosition, bool overViewport) {
        m_hovered = Handle::None;
        if(m_isShown && m_camera && overViewport && !m_manipulateds.empty()) {
            const auto point = m_camera -> convertPixelToCoords(mousePosition);
            if(m_operation == Operation::Rotate) {
                if(containsRotation(point))
                    m_hovered = Handle::Rotation;
            }
            else if(m_xAxisManipulator.contains(point))
                m_hovered = Handle::X;
            else if(m_yAxisManipulator.contains(point))
                m_hovered = Handle::Y;
            else if(m_XYAxisManipulator.contains(point))
                m_hovered = Handle::XY;
        }
        updateColors();
    }

    void Guizmo2D::updateHandleSizes() {
        if(m_camera) {
            const auto zero = m_camera -> convertPixelToCoords({0.f, 0.f});
            const auto x = m_camera -> convertPixelToCoords({1.f, 0.f}) - zero;
            const auto y = m_camera -> convertPixelToCoords({0.f, 1.f}) - zero;
            const robot2D::vec2f units{std::sqrt(x.dot(x)), std::sqrt(y.dot(y))};
            if(std::isfinite(units.x) && std::isfinite(units.y) && units.x > 0.f && units.y > 0.f)
                m_unitsPerPixel = units;
        }
        m_xAxisManipulator.setSize({10.f * m_unitsPerPixel.y, 50.f * m_unitsPerPixel.x});
        m_yAxisManipulator.setSize({10.f * m_unitsPerPixel.x, 50.f * m_unitsPerPixel.y});
        m_XYAxisManipulator.setSize({20.f * m_unitsPerPixel.x, 20.f * m_unitsPerPixel.y});
        m_rotateSprite.setSize({rotateSize.x * m_unitsPerPixel.x, rotateSize.y * m_unitsPerPixel.y});
    }

    void Guizmo2D::updateColors() {
        auto color = [this](const Manipulator& manipulator, Handle handle, robot2D::Color base) {
            if(m_leftMousePressed)
                return manipulator.active() ? activeColor : base;
            return m_hovered == handle ? hoverColor : base;
        };
        m_xAxisManipulator.setColor(color(m_xAxisManipulator, Handle::X, xColor));
        m_yAxisManipulator.setColor(color(m_yAxisManipulator, Handle::Y, yColor));
        m_XYAxisManipulator.setColor(color(m_XYAxisManipulator, Handle::XY, planeColor));
    }

    bool Guizmo2D::containsRotation(robot2D::vec2f point) const {
        const auto bounds = m_rotateSprite.getGlobalBounds();
        if(bounds.width <= 0.f || bounds.height <= 0.f)
            return false;
        const auto offset = point - bounds.centerPoint();
        const float x = offset.x / (bounds.width * 0.5f);
        const float y = offset.y / (bounds.height * 0.5f);
        const float distance = std::sqrt(x * x + y * y);
        return distance >= 0.76f && distance <= 1.16f;
    }

    void Guizmo2D::setIsShow(bool flag) {
        if(!flag) {
            m_hovered = Handle::None;
            cancelDrag();
        }
        m_isShown = flag;
    }

    void Guizmo2D::cancelDrag() {
        m_leftMousePressed = false;
        m_isCollide = false;
        m_xAxisManipulator.reset();
        m_yAxisManipulator.reset();
        m_XYAxisManipulator.reset();
        updateColors();
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
            const auto bounds = m_rotateSprite.getGlobalBounds();
            drawRing(target, states, bounds, m_unitsPerPixel, 4.f, {20.f, 25.f, 32.f, 230.f});
            const auto color = m_leftMousePressed ? activeColor
                    : (m_hovered == Handle::Rotation ? hoverColor : robot2D::Color{183.f, 205.f, 232.f});
            drawRing(target, states, bounds, m_unitsPerPixel, m_leftMousePressed ? 3.f : 2.f, color);
        }

    }


    void Guizmo2D::moveManipulators(const robot2D::vec2f& diff) {
        m_xAxisManipulator.moveX(diff.x);
        m_xAxisManipulator.moveY(diff.y);
        m_yAxisManipulator.moveX(diff.x);
        m_yAxisManipulator.moveY(diff.y);
        m_XYAxisManipulator.moveX(diff.x);
        m_XYAxisManipulator.moveY(diff.y);
    }


    namespace {

        float angleBetweenVectorInDegress(const robot2D::vec2f& center, const robot2D::vec2f& oldPos,
            const robot2D::vec2f& newPos) {
            robot2D::vec2f v1 = oldPos - center;
            robot2D::vec2f v2 = newPos - center;

            float dot = v1.dot(v2);
            float cross = v1.cross(v2);
            float angle = -std::atan2(cross, dot);
            return angle * 180 / M_PI;
        }

    }


    void Guizmo2D::setSnapping(bool enabled, float distance, float angle) {
        if(!std::isfinite(distance) || !std::isfinite(angle) || distance <= 0.f || angle <= 0.f)
            return;
        if(m_snapEnabled != enabled || m_snapDistance != distance || m_snapAngle != angle)
            cancelDrag();
        m_snapEnabled = enabled;
        m_snapDistance = distance;
        m_snapAngle = angle;
    }

    void Guizmo2D::processMouseMoved(robot2D::Event event) {
        if(!m_leftMousePressed) return;
        struct TransformState {
            robot2D::vec2f position;
            robot2D::vec2f size;
            float rotation;
        };
        std::vector<TransformState> previous;
        for(const auto* transform: m_manipulateds)
            previous.push_back({transform -> getPosition(), transform -> getSize(), transform -> getRotation()});
        robot2D::vec2f moveVector{event.move.x, event.move.y};
        moveVector = m_camera -> convertPixelToCoords(moveVector);
        if(m_snapEnabled && m_operation != Operation::Rotate) {
            auto offset = moveVector - m_dragStart;
            offset.x = std::round(offset.x / m_snapDistance) * m_snapDistance;
            offset.y = std::round(offset.y / m_snapDistance) * m_snapDistance;
            moveVector = m_dragStart + offset;
        }

        robot2D::vec2f diff{};
        switch(m_operation) {
        case Operation::Move: {
            if(m_xAxisManipulator.active()) {
                /// move transformAble by x
                diff.x = moveVector.x - m_manipulatorLastPos.x;
            }
            else if(m_yAxisManipulator.active()) {
                /// move transformAble by y
                diff.y = moveVector.y - m_manipulatorLastPos.y;
            }
            else if(m_XYAxisManipulator.active()) {
                diff = moveVector - m_manipulatorLastPos;
            }

            m_manipulatorLastPos += diff;
            moveManipulators(diff);


            for(auto transform: m_movementRoots) {
                auto pos = transform -> getPosition();
                transform -> setPosition(pos + diff);
            }

            break;
        }
        case Operation::Scale: {

            if(m_xAxisManipulator.active()) {
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
            }
            else if(m_yAxisManipulator.active()) {
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
            for(std::size_t index = 0; index < m_manipulateds.size(); ++index) {
                auto* transform = m_manipulateds[index];
                auto pos = transform -> getPosition();
                float diffAngle = angleBetweenVectorInDegress(pos, moveVector, m_manipulatorLastPos);
                if(m_snapEnabled) {
                    auto& remainder = m_rotationRemainders[index];
                    remainder += diffAngle;
                    diffAngle = std::round(remainder / m_snapAngle) * m_snapAngle;
                    remainder -= diffAngle;
                }
                transform -> rotate(diffAngle);
            }
            m_manipulatorLastPos = moveVector;
            break;
        }
        }
        for(std::size_t index = 0; index < m_manipulateds.size(); ++index) {
            const auto* transform = m_manipulateds[index];
            const auto& before = previous[index];
            if(before.position != transform -> getPosition() || before.size != transform -> getSize() ||
                before.rotation != transform -> getRotation()) {
                if(m_onModified)
                    m_onModified();
                break;
            }
        }
    }

    void Guizmo2D::processMousePressed(robot2D::Event event) {
        if(event.mouse.btn == robot2D::mouse2int(robot2D::Mouse::MouseLeft)) {
            cancelDrag();
            robot2D::vec2f mousePoint{static_cast<float>(event.mouse.x), static_cast<float>(event.mouse.y)};
            mousePoint = m_camera -> convertPixelToCoords(mousePoint);
            const bool hit = m_operation == Operation::Rotate ? containsRotation(mousePoint)
                    : (m_xAxisManipulator.isPressed(mousePoint)
                || m_yAxisManipulator.isPressed(mousePoint)
                || m_XYAxisManipulator.isPressed(mousePoint));
            if(hit) {
                m_leftMousePressed = true;
                m_manipulatorLastPos = mousePoint;
                m_dragStart = mousePoint;
                m_rotationRemainders.assign(m_manipulateds.size(), 0.f);
                m_isCollide = true;
            }
            else
                m_isCollide = false;
            updateColors();
        }
    }

    void Guizmo2D::processMouseRelease(robot2D::Event event) {
        if(event.mouse.btn == robot2D::mouse2int(robot2D::Mouse::MouseLeft)) {
            m_leftMousePressed = false;
            m_isCollide = false;
        }
    }


    void Guizmo2D::setManipulated(TransformComponent* transformable) {
        if(m_manipulateds.size() != (transformable ? 1u : 0u)
            || (transformable && m_manipulateds[0] != transformable))
            cancelDrag();
        m_manipulateds.clear();
        m_movementRoots.clear();
        if(!transformable)
            return;
        m_manipulateds.emplace_back(transformable);
        m_movementRoots = m_manipulateds;
        updateHandleSizes();

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

                const robot2D::vec2f xyOffset{3.f * m_unitsPerPixel.x, 3.f * m_unitsPerPixel.y};

                m_xAxisManipulator.setPosition({middle.x, middle.y + xSize.y / 2.f});
                m_yAxisManipulator.setPosition({middle.x + ySize.x / 2.F, middle.y});
                m_XYAxisManipulator.setPosition(robot2D::vec2f{middle.x + xyOffset.x,
                                                               middle.y - xySize.y - xyOffset.y});
            }
            else {
                auto bounds = transformable -> getGlobalBounds();

                auto xSize = m_xAxisManipulator.getSize();
                auto ySize = m_yAxisManipulator.getSize();
                auto xySize = m_XYAxisManipulator.getSize();

                const robot2D::vec2f xyOffset{3.f * m_unitsPerPixel.x, 3.f * m_unitsPerPixel.y};

                auto middle = bounds.centerPoint();

                m_xAxisManipulator.setPosition( {middle.x, middle.y + xSize.y / 2.f} );
                m_yAxisManipulator.setPosition( {middle.x + ySize.x / 2.F, middle.y} );
                m_XYAxisManipulator.setPosition( {middle.x + xyOffset.x, middle.y - xySize.y - xyOffset.y} );
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
        if(m_manipulateds != transformables)
            cancelDrag();
        m_manipulateds = transformables;
        m_movementRoots = transformables;
        if(transformables.empty())
            return;
        updateHandleSizes();
        robot2D::vec2f minLeftPoint = transformables[0] -> getPosition();
        robot2D::vec2f maxRightPoint = transformables[0] -> getPosition();

        for(auto transform: transformables) {
            auto position = transform -> getPosition();
            if(position.x < minLeftPoint.x)
                minLeftPoint.x = position.x;
            if(position.y < minLeftPoint.y)
                minLeftPoint.y = position.y;
            if(position.x > maxRightPoint.x)
                maxRightPoint.x = position.x;
            if(position.y > maxRightPoint.y)
                maxRightPoint.y = position.y;
        }

        const robot2D::vec2f xyOffset{3.f * m_unitsPerPixel.x, 3.f * m_unitsPerPixel.y};

        robot2D::vec2f distance = {maxRightPoint.x - minLeftPoint.x, maxRightPoint.y - minLeftPoint.y};

        auto middle = robot2D::vec2f{ minLeftPoint.x + distance.x / 2.f,
                                      minLeftPoint.y + distance.y / 2.f};
        auto xSize = m_xAxisManipulator.getSize();
        auto ySize = m_yAxisManipulator.getSize();
        auto xySize = m_XYAxisManipulator.getSize();

        m_xAxisManipulator.setPosition({middle.x, middle.y + xSize.y / 2.f});
        m_yAxisManipulator.setPosition({middle.x + ySize.x / 2.F, middle.y});
        m_XYAxisManipulator.setPosition({middle.x + xyOffset.x, middle.y - xySize.y - xyOffset.y});
    }

    void Guizmo2D::setMovementRoots(const std::vector<TransformComponent*>& transforms) {
        m_movementRoots = transforms;
    }

    void Guizmo2D::setOperationType(Guizmo2D::Operation type) {
        if(type != m_operation) {
            cancelDrag();
            m_hovered = Handle::None;
        }
        m_operation = type;
        m_xAxisManipulator.m_sprite.setRotate(270);
        m_yAxisManipulator.m_sprite.setRotate(180);
        m_xAxisManipulator.setScaleHandle(type == Operation::Scale);
        m_yAxisManipulator.setScaleHandle(type == Operation::Scale);
        updateColors();
    }

    void Guizmo2D::setCamera(IEditorCamera::Ptr camera) {
        m_camera = std::move(camera);
        setOperationType(m_operation);
    }

}
