#pragma once

#include "../sprite-animation/AnimatedSprite.hpp"
#include "Camera.hpp"
#include <array>
#include <optional>
#include <robot2D/Core/Mouse.hpp>

class CameraSprite : public AnimatedSprite {
public:
    void setup() override {
        AnimatedSprite::setup();
        sprite().setPosition({1000.F, 600.F});
        for(unsigned i = 0; i < m_landmarks.size(); ++i) {
            auto& landmark = m_landmarks[i];
            landmark.setTexture(*sprite().getTexture(), {0, 0, 64, 64});
            landmark.setSize({64.F, 64.F});
            landmark.setPosition({100.F + (i % 5) * 420.F, 100.F + (i / 5) * 450.F});
        }
    }

    void handleEvents(const robot2D::Event& event) override {
        AnimatedSprite::handleEvents(event);
        if(event.getIf<robot2D::Event::LostFocus>()) m_click.reset();
        if(const auto* click = event.getIf<robot2D::Event::MousePressed>()) {
            if(click -> btn == robot2D::mouse2int(robot2D::Mouse::MouseLeft))
                m_click = robot2D::vec2f{click -> x, click -> y};
        }
    }

    void update(float deltaTime) override {
        // World bounds stay fixed; the camera no longer defines the movement limits.
        moveInside(deltaTime, {2000.F, 1200.F});
        advanceAnimation(deltaTime);
    }

    void render() override {
        const auto metrics = m_window -> getMetrics();
        if(!metrics.drawable()) {
            m_click.reset();
            return;
        }
        followPlayer(m_camera, sprite().getPosition(), metrics.logicalSize);
        m_window -> setView(m_camera);
        // Engine synchronized the native surface before render(). Map using this frame's view.
        if(m_click) {
            sprite().setPosition(m_window -> mapClientToCoords(*m_click, m_camera));
            keepInside(sprite(), {2000.F, 1200.F});
            m_click.reset();
        }
        m_window -> clear(robot2D::Color(22.F, 32.F, 40.F));
        m_window -> beforeRender();
        for(const auto& landmark : m_landmarks) m_window -> draw(landmark);
        m_window -> draw(sprite());
        m_window -> afterRender();
        m_window -> display();
    }

private:
    robot2D::View m_camera;
    std::array<robot2D::Sprite, 15> m_landmarks;
    std::optional<robot2D::vec2f> m_click;
};
