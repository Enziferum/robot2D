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

#include <fstream>
#include <filesystem>
#include <yaml-cpp/yaml.h>

#include <robot2D/Core/Clock.hpp>
#include <editor/Application.hpp>
#include <editor/EditorStyles.hpp>
#include <editor/ProjectSerializer.hpp>
#include <editor/Utils.hpp>
#include <editor/EventBinder.hpp>

namespace editor {
    //make autotranslator son of bitches from middle east
    Application::Application():
            robot2D::Application(),
            m_appConfiguration{},
            m_editor{m_messageBus},
            m_state{State::ProjectInspector},
            m_configuration{},
            m_editorCache{m_configuration},
            m_projectManager{m_configuration},
            m_projectInspector{m_editorCache}
             {}

    void Application::setup() {

        {
            robot2D::Image iconImage;
            iconImage.loadFromFile(m_appConfiguration.logoPath);
            m_window -> setIcon(std::move(iconImage));
        }

        m_guiWrapper.init(*m_window);
        m_editor.setup(m_window);

        auto [status, result] = m_configuration.getValue(ConfigurationKey::CachePath);

        if(!m_editorCache.parseCache(result)) {
            if(m_editorCache.getError() == EditorCacheError::NoCacheFile){}
            else {
                RB_EDITOR_ERROR("Error to parse Editor Cache, description := {0}",
                                errorToString(m_editorCache.getError()));
                return;
            }
        }

        if(m_editorCache.isShowInspector())
            m_state = State::ProjectInspector;
        else
            m_state = State::Editor;

        if(m_state == State::ProjectInspector) {
            auto windowSize = m_window -> getSize();
            auto monitorSize = m_window -> getMonitorSize();
            auto centerPoint = getCenterPoint(windowSize, monitorSize);

            m_window -> setPosition(centerPoint);
            m_window -> setResizable(false);
            m_window -> setSize(m_appConfiguration.inspectorSize);

            applyStyle(EditorStyle::GoldBlack);


            m_projectInspector.addCallback(ProjectInspector::CallbackType::Create,
                                           [this](ProjectDescription project) {
                                               createProject(std::move(project));
                                           });
            m_projectInspector.addCallback(ProjectInspector::CallbackType::Load,
                                           [this](ProjectDescription project) {
                                               loadProject(std::move(project));
                                           });
            m_projectInspector.addCallback(ProjectInspector::CallbackType::Delete,
                                           [this](ProjectDescription project) {
                                               deleteProject(std::move(project));
                                           });

            m_projectInspector.setup(m_window);
        } else {
            const auto& projectDescription = m_editorCache.getCurrentProject();
            if(projectDescription.empty())
                RB_EDITOR_WARN("projectDescription.empty()");

            if(!m_projectManager.load(projectDescription)) {
                RB_EDITOR_ERROR("ProjectManager can't load Project := {0}",
                                errorToString(m_projectManager.getError()));
                return;
            }
            auto project = m_projectManager.getCurrentProject();
            m_editor.loadProject(project);
        }
    }

    void Application::handleEvents(const robot2D::Event& event) {
        EventBinder eventBinder{event};

        eventBinder.Dispatch(robot2D::Event::Resized,
                                 [this](const robot2D::Event& evt) {
            RB_EDITOR_INFO("New Size = {0} and {1}", evt.size.widht, evt.size.heigth);
            m_window -> resize({static_cast<int>(evt.size.widht),
                                static_cast<int>(evt.size.heigth)});
            m_window -> setView({{0, 0}, {evt.size.widht, evt.size.heigth}});
        });

        eventBinder.Dispatch(robot2D::Event::KeyPressed,
                                 [this](const robot2D::Event& evt) {
            if(evt.key.code == robot2D::Key::ESCAPE)
                m_running = false;
        });

        m_guiWrapper.handleEvents(event);
        if(m_state == State::Editor)
            m_editor.handleEvents(event);
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

    void Application::guiUpdate(float dt) {
        m_guiWrapper.update(dt);
    }

    void Application::render() {
        m_window -> clear();
        if(m_state == State::Editor)
            m_editor.render();
        else if(m_state == State::ProjectInspector)
            m_projectInspector.render();
        m_guiWrapper.render();
        m_window -> display();
    }


    void Application::createProject(ProjectDescription&& projectDescription) {
        if(!m_editorCache.addProject(projectDescription)) {
            RB_EDITOR_ERROR("Can't add Project to Cache := {0}",
                            errorToString(m_editorCache.getError()));
            return;
        }

        if(!m_projectManager.add(projectDescription)) {
            RB_EDITOR_ERROR("Can't create Project := {0}",
                            errorToString(m_projectManager.getError()));
            return;
        }

        auto project = m_projectManager.getCurrentProject();

        m_editor.createProject(project);
        m_window -> setResizable(true);
        m_state = State::Editor;
    }

    void Application::deleteProject(ProjectDescription&& projectDescription) {

        if(!m_editorCache.removeProject(projectDescription)) {
            RB_EDITOR_ERROR("EditorCache can't delete Project := {0}",
                            errorToString(m_editorCache.getError()));
            return;
        }

        if(!m_projectManager.remove(projectDescription)) {
            RB_EDITOR_ERROR("ProjectManager can't delete Project := {0}",
                            errorToString(m_projectManager.getError()));
            return;
        }
    }

    void Application::loadProject(ProjectDescription&& projectDescription) {
        if(!m_editorCache.loadProject(projectDescription)) {
            RB_EDITOR_ERROR("Cache can't load Project := {0}", errorToString(m_editorCache.getError()));
            return;
        }

        if(!m_projectManager.load(projectDescription)) {
            RB_EDITOR_ERROR("ProjectManager can't load Project := {0}", errorToString(m_projectManager.getError()));
            return;
        }

        m_state = State::Editor;
        m_window -> setResizable(true);
        m_editor.loadProject(m_projectManager.getCurrentProject());
    }


}