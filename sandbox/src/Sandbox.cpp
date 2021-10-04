/*********************************************************************
(c) Alex Raag 2021
https://github.com/Enziferum
robot2D - Zlib license.
This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <robot2D/Core/Clock.hpp>
#include "sandbox/Sandbox.hpp"

namespace {
    robot2D::Clock m_frameClock;
    constexpr float timePerFrame = 1.F / 60.F;
    float processedTime = 0.F;
}

Sandbox::Sandbox(robot2D::RenderWindow& window):
    m_window(window), m_scene(nullptr) {

}

void Sandbox::setScene(Scene::Ptr scene) {
    m_scene = std::move(scene);
}

void Sandbox::run() {
    m_frameClock.restart();
    while (m_window.isOpen()) {
        float elapsed = m_frameClock.restart().asSeconds();
        processedTime += elapsed;
        while (processedTime > timePerFrame) {
            processedTime -= timePerFrame;
            handleEvents();
            update(timePerFrame);
        }
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
    m_window.clear(robot2D::Color::White);
    m_scene -> render();
    m_window.display();
}
