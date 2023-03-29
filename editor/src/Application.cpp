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

#include <editor/Application.hpp>
#include <editor/scripting/ScriptingEngine.hpp>

#include <editor/EditorStyles.hpp>
#include <editor/Utils.hpp>
#include <editor/EventBinder.hpp>
#include "panels/ImGuizmo.h"

namespace editor {
    Application::Application():
            robot2D::Application(),
            m_appConfiguration{},
            m_messageDispatcher{},
            m_logic{m_messageDispatcher},
            m_taskQueue{},
            m_guiWrapper{},
            m_editor{m_messageBus, m_guiWrapper},
            m_editorLogic(m_messageBus, m_messageDispatcher, m_taskQueue),
            m_projectInspector{m_messageBus}
            {}

    void Application::setup() {
        {
            robot2D::Image iconImage;
            iconImage.loadFromFile(m_appConfiguration.logoPath);
            m_window -> setIcon(std::move(iconImage));
        }

        m_editorLogic.setIEditor(&m_editor);
        m_logic.setup(&m_editorLogic);
        m_guiWrapper.setup(*m_window);
        m_editor.setup(m_window, &m_editorLogic);


        //// Load C# Mono
        ScriptEngine::Init();

        if(m_logic.getState() == AppState::ProjectInspector) {
            auto windowSize = m_window -> getSize();
            auto monitorSize = m_window -> getMonitorSize();
            auto centerPoint = getCenterPoint(windowSize, monitorSize);

            m_window -> setPosition(centerPoint);
            m_window -> setResizable(false);
            m_window -> setSize(m_appConfiguration.inspectorSize);

            applyStyle(EditorStyle::GoldBlack);
            m_projectInspector.setup(m_window, m_logic.getCache().getProjects());
        }

        m_eventBinder.bindEvent(robot2D::Event::Resized, [this](const robot2D::Event& evt) {
            RB_EDITOR_INFO("New Size = {0} and {1}", evt.size.widht, evt.size.heigth);
            m_window -> resize({static_cast<int>(evt.size.widht),
                                static_cast<int>(evt.size.heigth)});
            m_window -> setView({{0, 0}, {evt.size.widht, evt.size.heigth}});
        });

        m_eventBinder.bindEvent(robot2D::Event::KeyPressed, [this](const robot2D::Event& evt) {
            if(evt.key.code == robot2D::Key::ESCAPE)
                m_running = false;
        });
    }

    void Application::handleEvents(const robot2D::Event& event) {
        m_eventBinder.handleEvents(event);

        m_guiWrapper.handleEvents(event);
        if(m_logic.getState() == AppState::Editor)
            m_editor.handleEvents(event);
    }

    void Application::handleMessages() {
        robot2D::Message message{};
        while (m_messageBus.pollMessages(message)) {
            if(m_logic.getState() == AppState::Editor)
                m_editor.handleMessages(message);

            m_messageDispatcher.process(message);
        }
    }

    void Application::update(float dt) {
        m_taskQueue.process();
        if(m_logic.getState() == AppState::Editor)
            m_editor.update(dt);
    }

    void Application::guiUpdate(float dt) {
        m_guiWrapper.update(dt);
        ImGuizmo::BeginFrame();
    }

    void Application::render() {
        m_window -> clear();
        if(m_logic.getState() == AppState::Editor)
            m_editor.render();
        else if(m_logic.getState() == AppState::ProjectInspector)
            m_projectInspector.render();
        m_guiWrapper.render();
        m_window -> display();
    }

    void Application::destroy() {
       ScriptEngine::Shutdown();
    }
}