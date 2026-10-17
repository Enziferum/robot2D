// Extends lesson 01: resource loading and rendering stay in FirstSprite.
#pragma once

#include "../first-sprite/FirstSprite.hpp"
#include "KeyboardMovement.hpp"

class MovingSprite : public FirstSprite {
public:
    using FirstSprite::FirstSprite;

    void handleEvents(const robot2D::Event& event) override {
        FirstSprite::handleEvents(event); // Preserve the window-close behaviour.
        m_input.handleEvent(event);
        if(const auto* pressed = event.getIf<robot2D::Event::KeyPressed>()) {
            if(pressed -> code == robot2D::Key::ESCAPE)
                m_running = false;
        }
    }

    void update(float deltaTime) override {
        sprite().move(m_input.displacement(deltaTime));
    }

private:
    KeyboardMovement m_input;
};
