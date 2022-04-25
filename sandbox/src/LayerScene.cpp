#include <sandbox/LayerScene.hpp>


LayerScene::LayerScene(robot2D::RenderWindow& window) : Scene(window) {

}


LayerScene::~LayerScene() {

}

void LayerScene::setup() {

}

void LayerScene::handleEvents(const robot2D::Event& event) {

}

void LayerScene::update(float dt) {

}

void LayerScene::imGuiRender() {

}

void LayerScene::render() {
    for(const auto& layer: m_layers)
        m_window.draw(layer);
}
