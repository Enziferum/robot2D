#pragma once

#include "../first-sprite/FirstSprite.hpp"
#include "../keyboard-movement/KeyboardMovement.hpp"
#include "Collision.hpp"
#include <array>

class CollisionSprite : public FirstSprite {
public:
    using FirstSprite::FirstSprite;

    void setup() override {
        FirstSprite::setup();
        const std::array<unsigned char, 4> white{255, 255, 255, 255};
        m_solidTexture.create({1, 1}, white.data());
        configureGeometry();
        for(auto& wall : m_walls) {
            wall.setTexture(m_solidTexture, {0, 0, 1, 1});
            wall.setColor(robot2D::Color(68.F, 133.F, 139.F));
        }
        m_hitbox.setTexture(m_solidTexture, {0, 0, 1, 1});
        m_hitbox.setSize(sprite().getSize());
        m_hitbox.setOrigin(sprite().getOrigin());
    }

    void handleEvents(const robot2D::Event& event) override {
        FirstSprite::handleEvents(event);
        m_input.handleEvent(event);
        if(const auto* key = event.getIf<robot2D::Event::KeyPressed>()) {
            if(key -> code == robot2D::Key::ESCAPE) m_running = false;
        }
    }

    void update(float deltaTime) override {
        if(m_window -> getMetrics().drawable()) simulate(deltaTime);
    }

    void render() override {
        if(!m_window -> getMetrics().drawable()) return;
        m_window -> clear(robot2D::Color(22.F, 32.F, 40.F));
        m_window -> beforeRender();
        for(const auto& wall : m_walls) m_window -> draw(wall);
        m_hitbox.setPosition(sprite().getPosition());
        m_hitbox.setColor(m_contact.blockedX || m_contact.blockedY
            ? robot2D::Color(230.F, 174.F, 76.F) : robot2D::Color(35.F, 65.F, 75.F));
        m_window -> draw(m_hitbox); // Makes the complete collision quad visible.
        m_window -> draw(sprite());
        drawExtras();
        m_window -> afterRender();
        m_window -> display();
    }

    void destroy() override {
        m_solidTexture.release();
        FirstSprite::destroy();
    }

protected:
    void clearMovement() { m_input.clearHeld(); }

    // Later lessons can add triggers/HUD inside the same render pass.
    virtual void drawExtras() {}

    void configureGeometry() {
        sprite().setSize({96.F, 96.F});
        sprite().setOrigin({0.5F, 0.5F});
        sprite().setPosition({100.F, 100.F});
        const std::array<robot2D::FloatRect, 7> rectangles{{
            {0.F, 0.F, 800.F, 24.F}, {0.F, 576.F, 800.F, 24.F},
            {0.F, 24.F, 24.F, 552.F}, {776.F, 24.F, 24.F, 552.F},
            {360.F, 170.F, 40.F, 260.F}, {150.F, 260.F, 120.F, 40.F},
            {530.F, 360.F, 150.F, 40.F}}};
        for(unsigned i = 0; i < m_walls.size(); ++i) {
            m_walls[i].setPosition({rectangles[i].lx, rectangles[i].ly});
            m_walls[i].setSize({rectangles[i].width, rectangles[i].height});
        }
    }

    void simulate(float deltaTime) {
        m_contact = moveAndSlide(sprite(), m_input.displacement(deltaTime), m_walls);
    }

private:
    KeyboardMovement m_input;
    robot2D::Texture m_solidTexture;
    std::array<robot2D::Sprite, 7> m_walls;
    robot2D::Sprite m_hitbox;
    CollisionResult m_contact;
};
