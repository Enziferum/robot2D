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
#include <editor/Application.hpp>

namespace editor {

    namespace {
        robot2D::Clock frameClock;
        constexpr float timePerFrame = 1.F / 60.F;
        float processedTime = 0.F;
    }

    Application::Application(): m_window({1280, 920}, "Editor", robot2D::WindowContext::Default),
                                m_editor{m_window} {}

    void Application::run() {
        setup();
        frameClock.restart();
        while(m_window.isOpen()) {
            float elapsed = frameClock.restart().asSeconds();
            processedTime += elapsed;
            while (processedTime > timePerFrame) {
                processedTime -= timePerFrame;
                handleEvents();
                handleMessages();
                update(timePerFrame);
            }
            m_guiWrapper.update(elapsed);
            render();
        }
    }

    void Application::setup() {
        m_guiWrapper.init(m_window);
        m_editor.setup();
    }

    void Application::handleEvents() {
        robot2D::Event event{};
        while (m_window.pollEvents(event)) {
            if(event.type == robot2D::Event::KeyPressed &&
            event.key.code == robot2D::Key::ESCAPE)
                m_window.close();
            m_guiWrapper.handleEvents(event);
            m_editor.handleEvents(event);
        }
    }

    void Application::handleMessages() {
        robot2D::Message message{};
        while (m_messageBus.pollMessages(message)) {
            m_editor.handleMessages(message);
        }
    }

    void Application::update(float dt) {
        m_editor.update(dt);
    }

    void Application::render() {
        m_window.clear();
        m_editor.render();
        m_guiWrapper.render();
        m_window.display();
    }

}