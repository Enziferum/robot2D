// robot2D Core lesson 02. See the included Zlib license.
#pragma once

#include <array>
#include <cmath>
#include <robot2D/Core/Event.hpp>
#include <robot2D/Core/Vector2.hpp>

class KeyboardMovement {
public:
    void handleEvent(const robot2D::Event& event) {
        if(event.getIf<robot2D::Event::LostFocus>()) {
            m_held.fill(false);
            m_focused = false;
        }
        else if(event.getIf<robot2D::Event::GainFocus>()) {
            m_focused = true;
        }
        else if(const auto* pressed = event.getIf<robot2D::Event::KeyPressed>()) {
            if(m_focused && !pressed -> repeat)
                setHeld(pressed -> code, true);
        }
        else if(const auto* released = event.getIf<robot2D::Event::KeyReleased>()) {
            setHeld(released -> code, false);
        }
    }

    robot2D::vec2f displacement(float deltaTime) const {
        if(!m_focused || deltaTime <= 0.F || !std::isfinite(deltaTime))
            return {};

        const bool left = m_held[0] || m_held[4];
        const bool right = m_held[1] || m_held[5];
        const bool up = m_held[2] || m_held[6];
        const bool down = m_held[3] || m_held[7];
        robot2D::vec2f direction{
            static_cast<float>(right) - static_cast<float>(left),
            static_cast<float>(down) - static_cast<float>(up)};
        const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if(length == 0.F)
            return {};

        constexpr float speed = 180.F; // World units per second, not per frame.
        return direction * (speed * deltaTime / length);
    }

private:
    void setHeld(robot2D::Key key, bool held) {
        using robot2D::Key;
        switch(key) {
            case Key::A: m_held[0] = held; break;
            case Key::D: m_held[1] = held; break;
            case Key::W: m_held[2] = held; break;
            case Key::S: m_held[3] = held; break;
            case Key::LEFT: m_held[4] = held; break;
            case Key::RIGHT: m_held[5] = held; break;
            case Key::UP: m_held[6] = held; break;
            case Key::DOWN: m_held[7] = held; break;
            default: break;
        }
    }

    // Track aliases separately: releasing A must not release a held Left arrow.
    std::array<bool, 8> m_held{};
    bool m_focused = true;
};
