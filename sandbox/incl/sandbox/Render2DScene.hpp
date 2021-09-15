//
// Created by Necromant on 15.09.2021.
//

#pragma once

#include "Scene.hpp"

// Scene for Implementing BatchRender
class Render2DScene: public Scene {
public:
    explicit Render2DScene(robot2D::RenderWindow&);
    ~Render2DScene() override = default;

    virtual void handleEvents(const robot2D::Event &event) override;
    virtual void update(float dt) override;
    virtual void imGuiRender() override;
    virtual void render() override;
};