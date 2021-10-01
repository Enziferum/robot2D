//
// Created by Necromant on 15.09.2021.
//

#include "sandbox/Sandbox.hpp"

Sandbox::Sandbox(robot2D::RenderWindow& window):
    m_window(window), m_scene(nullptr) {

}

void Sandbox::setScene(Scene::Ptr scene) {
    m_scene = std::move(scene);
}

void Sandbox::run() {
    while (m_window.isOpen()) {
        handleEvents();
        update(0.f);
        render();
    }
}

void Sandbox::handleEvents() {
    robot2D::Event event{};
    while (m_window.pollEvents(event)) {
        if(event.type == robot2D::Event::KeyPressed &&
            event.key.code == robot2D::Key::ESCAPE)
            m_window.close();
        m_scene -> handleEvents(event);
    }
}

void Sandbox::update(float dt) {
    m_scene -> update(dt);
}

void Sandbox::render() {
    m_window.clear(robot2D::Color::Black);
    m_scene -> render();
    m_window.display();
}
