//
// Created by Necromant on 15.09.2021.
//

#pragma once

#include <memory>
#include <robot2D/Graphics/RenderWindow.hpp>

/// Basic Scene for our Sandbox Case
class Scene {
public:
    using Ptr = std::unique_ptr<Scene>;
public:
    Scene(robot2D::RenderWindow&);
    virtual ~Scene() = 0;

    virtual void handleEvents(const robot2D::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void imGuiRender() = 0;
    virtual void render() = 0;

protected:
    robot2D::RenderWindow& m_window;
};
