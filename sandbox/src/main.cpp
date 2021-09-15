//
// Created by Necromant on 15.09.2021.
//
#include <memory>
#include <robot2D/Graphics/RenderWindow.hpp>
#include "sandbox/Sandbox.hpp"
#include "sandbox/Render2DScene.hpp"

int main() {
    robot2D::RenderWindow window{{1280, 920}, {"Sandbox"}, robot2D::WindowContext::Default};

    Sandbox sandbox(window);

    //// Put own scenes here /////
    Render2DScene::Ptr render2DScene = std::make_unique<Render2DScene>(window);

    //// Put own scenes here /////

    sandbox.setScene(std::move(render2DScene));
    sandbox.run();

    return 0;
}