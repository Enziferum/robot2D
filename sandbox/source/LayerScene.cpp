#include <sandbox/LayerScene.hpp>

LayerScene::LayerScene(robot2D::RenderWindow& window): Scene(window) {

}

LayerScene::~LayerScene() {}

void LayerScene::setup() {

}

void LayerScene::handleEvents(const robot2D::Event& event) {
    (void) event;
}

void LayerScene::update(float dt) {
    (void)dt;
}

void LayerScene::imGuiRender() {

}

void LayerScene::render() {
    m_window.beforeRender();
    m_window.afterRender();
}
