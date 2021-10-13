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

namespace editor {

    namespace {
        robot2D::Clock frameClock;
        constexpr float timePerFrame = 1.F / 60.F;
        float processedTime = 0.F;

        const std::string editorCachePath = "editor.cache";
        const std::string editorVersion = "0.1";
        const std::string logoPath = "res/textures/logo.png";

        const robot2D::vec2u inspectorSize = {600, 400};

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
            m_state{State::ProjectInspector},
            m_projectInspector{m_window, m_editorCache} {}



    void Application::setup() {
        {
            robot2D::Image iconImage;
            iconImage.loadFromFile(logoPath);
            m_window.setIcon(std::move(iconImage));
        }
        m_guiWrapper.init(m_window);
        m_editor.setup();

        if(!m_editorCache.parseCache(editorCachePath)) {
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
            auto windowSize = m_window.getSize();
            auto monitorSize = m_window.getMonitorSize();
            auto centerPoint = getCenterPoint(windowSize, monitorSize);
            m_window.setPosition(centerPoint);

            //todo setWindow not to be resizable
            //todo subscribeLogic

            m_window.setSize(inspectorSize);
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
            m_projectInspector.setup();
        } else {
            const auto& projectDescription = m_editorCache.getCurrentProject();
            if(projectDescription.empty())
                RB_EDITOR_WARN("projectDescription.empty()");

            if(!m_projectManager.load(projectDescription)) {
                RB_EDITOR_ERROR("ProjectManager can't load Project := {0}", errorToString(m_projectManager.getError()));
                return;
            }
            auto project = m_projectManager.getCurrentProject();
            m_editor.loadProject(project);
        }

    }

    void Application::run() {
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
        else if(m_state == State::ProjectInspector)
            m_projectInspector.render();
        m_guiWrapper.render();
        m_window.display();
    }


    void Application::createProject(ProjectDescription projectDescription) {
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
        m_state = State::Editor;
        m_window.setSize(defaultWindowSize);
    }

    void Application::deleteProject(ProjectDescription projectDescription) {

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

    void Application::loadProject(ProjectDescription projectDescription) {
        if(!m_editorCache.loadProject(projectDescription)) {
            RB_EDITOR_ERROR("Cache can't load Project := {0}", errorToString(m_editorCache.getError()));
            return;
        }

        if(!m_projectManager.load(projectDescription)) {
            RB_EDITOR_ERROR("ProjectManager can't load Project := {0}", errorToString(m_projectManager.getError()));
            return;
        }

        m_state = State::Editor;
        m_window.setSize(defaultWindowSize);
        m_editor.loadProject(m_projectManager.getCurrentProject());
    }
}