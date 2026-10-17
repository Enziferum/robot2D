// robot2D Core lesson 01. Distributed under the repository Zlib license.
#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <robot2D/Engine.hpp>
#include <robot2D/Graphics/Sprite.hpp>
#include <robot2D/Graphics/Texture.hpp>

class FirstSprite : public robot2D::Application {
public:
    explicit FirstSprite(std::string texturePath = "assets/robot.png") :
        m_texturePath(std::move(texturePath)) {}

    void setup() override {
        // setup() runs after Engine creates the window and its GL context.
        if(!m_texture.loadFromFile(m_texturePath)) {
            throw std::runtime_error("Cannot load texture: " +
                std::filesystem::absolute(m_texturePath).string());
        }

        const auto size = m_texture.getSize();
        m_sprite.setTexture(m_texture, robot2D::IntRect(0, 0,
            static_cast<int>(size.x), static_cast<int>(size.y)));
        m_sprite.setSize({256.F, 256.F});
        m_sprite.setPosition({272.F, 172.F});
        m_running = true;
    }

    void handleEvents(const robot2D::Event& event) override {
        if(event.getIf<robot2D::Event::Closed>())
            m_running = false;
    }

    void render() override {
        m_window -> clear(robot2D::Color(22.F, 32.F, 40.F));
        m_window -> beforeRender();
        m_window -> draw(m_sprite);
        m_window -> afterRender();
        m_window -> display();
    }

    void destroy() override {
        // Engine calls this while the window/context is still alive.
        m_texture.release();
    }

protected:
    robot2D::Sprite& sprite() { return m_sprite; }

private:
    std::string m_texturePath;
    robot2D::Texture m_texture;
    robot2D::Sprite m_sprite; // Borrows m_texture; never owns or copies it.
};
