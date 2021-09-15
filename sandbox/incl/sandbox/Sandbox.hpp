//
// Created by Necromant on 15.09.2021.
//

#pragma once

#include <robot2D/Graphics/RenderWindow.hpp>
#include "Scene.hpp"

class Sandbox {
public:
    Sandbox(robot2D::RenderWindow& window);
    ~Sandbox() = default;

    void setScene(Scene::Ptr scene);
    void run();
private:
    void handleEvents();
    void update(float dt);
    void render();

private:
    robot2D::RenderWindow& m_window;
    Scene::Ptr m_scene;
};
