//
// Created by Necromant on 15.09.2021.
//

#include "sandbox/Render2DScene.hpp"

robot2D::Texture texture;

struct Quad: public robot2D::Transformable, public robot2D::Drawable{
    void draw(robot2D::RenderTarget &target, robot2D::RenderStates states) const override {
        std::vector<robot2D::Vertex> vertices;
        vertices.push_back(robot2D::Vertex{{}, {}, robot2D::Color::Blue});
        vertices.push_back(robot2D::Vertex{{}, {}, robot2D::Color::Blue});
        vertices.push_back(robot2D::Vertex{{}, {}, robot2D::Color::Blue});
        vertices.push_back(robot2D::Vertex{{}, {}, robot2D::Color::Blue});
        states.transform *= getTransform();
        states.color = robot2D::Color::Blue;
        states.texture = nullptr;
        target.draw(vertices, states);
    }
};

Render2DScene::Render2DScene(robot2D::RenderWindow& window) : Scene(window){
    texture.loadFromFile("BossWalk.png");
}

void Render2DScene::handleEvents(const robot2D::Event& event) {

}

void Render2DScene::update(float dt) {

}

void Render2DScene::imGuiRender() {

}

void Render2DScene::render() {
    m_window.beforeRender();

    Quad quad;
    quad.setPosition({400.F, 400.F});
    quad.scale({100.F, 100.F});
    m_window.draw(quad);

    robot2D::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setPosition({300.F, 300.F});
    sprite.setSize({150.F, 150.F});

    sprite.setTextureRect({48, 82, 83, 91});
    m_window.draw(sprite);

    m_window.afterRender();
    m_window.flushRender();
}
