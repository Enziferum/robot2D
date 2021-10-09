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
#include <editor/EditorStyles.hpp>
namespace editor {

    namespace {
        robot2D::Clock frameClock;
        constexpr float timePerFrame = 1.F / 60.F;
        float processedTime = 0.F;

        robot2D::vec2u getCenterPoint(const robot2D::vec2u& objectSize, const robot2D::vec2u& frameSize) {
            robot2D::vec2u normalCenter = {frameSize.x / 2, frameSize.y / 2};
            robot2D::vec2u objectHalfSize = {objectSize.x / 2, objectSize.y / 2};
            return {normalCenter.x - objectHalfSize.x, normalCenter.y - objectHalfSize.y};
        }
    }


    Application::Application():
    defaultWindowSize{1280, 920},
    m_window(defaultWindowSize, "Editor", robot2D::WindowContext::Default),
                                m_editor{m_window, m_messageBus},
                                m_state{State::CreateProject},
                                m_projectCreator{m_window} {}

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

        if(m_state == State::CreateProject) {
            /// Create Project Stuff ///

            /// centeraize window ///
            auto windowSize = m_window.getSize();
            auto monitorSize = m_window.getMonitorSize();
            auto centerPoint = getCenterPoint(windowSize, monitorSize);
            m_window.setPosition(centerPoint);
            auto windowPos = m_window.getPosition();
            RB_EDITOR_INFO("Target Monitor size = {0}", monitorSize);
            RB_EDITOR_INFO("Window Position = {0}", windowPos);

            //todo setWindow not to be resizable

            m_window.setSize({600, 400});
            applyStyle(EditorStyle::GoldBlack);

            m_projectCreator.setup([this](std::string path) {
                m_window.setSize(defaultWindowSize);
                m_editor.createProject(path);
                m_state = State::Editor;
            },[this](std::string path) {
                m_editor.deleteProject(path);
            });
            /// Create Project Stuff ///
        } else {
            m_editor.setup();
        }
    }

    void Application::handleEvents() {
        robot2D::Event event{};
        while (m_window.pollEvents(event)) {
            if(event.type == robot2D::Event::Resized) {
                RB_EDITOR_INFO("New Size = {0} and {1}", event.size.widht, event.size.heigth);
                m_window.resize({event.size.widht, event.size.heigth});
                //robot2D::View{0, 0, event.size.widht, event.size.heigth}
                m_window.setView({{0, 0}, {event.size.widht, event.size.heigth}});
            }

            if(event.type == robot2D::Event::KeyPressed &&
            event.key.code == robot2D::Key::ESCAPE)
                m_window.close();
            m_guiWrapper.handleEvents(event);
            if(m_state == State::Editor)
                m_editor.handleEvents(event);
        }
    }

    void Application::handleMessages() {
        robot2D::Message message{};
        while (m_messageBus.pollMessages(message)) {
            if(m_state == State::Editor)
                m_editor.handleMessages(message);
        }
    }

    void Application::update(float dt) {
        if(m_state == State::Editor)
            m_editor.update(dt);
    }

    void Application::render() {
        m_window.clear();
        if(m_state == State::Editor)
            m_editor.render();
        else if(m_state == State::CreateProject)
            m_projectCreator.render();
        m_guiWrapper.render();
        m_window.display();
    }

    void Application::createProject(std::string path) {
        m_editor.createProject(path);
        m_state = State::Editor;
    }

    void Application::deleteProject(std::string path) {
        m_editor.deleteProject(path);
    }

}