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

        if(!std::filesystem::exists(editorCachePath)) {
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "Robot2D Editor" << YAML::Value << editorVersion;
            out << YAML::EndMap;

            std::ofstream ofstream(editorCachePath);
            if(!ofstream.is_open()) {
                RB_EDITOR_ERROR("Can't create Cache file");
                return;
            }

            ofstream << out.c_str();
            ofstream.close();
            m_state = State::Editor;
        } else {
            parseProjectCache();
        }

        if(m_state == State::CreateProject) {
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

            std::vector<ProjectDescription> projectDescriptions;
            for(auto& it: m_projectsCache) {
                projectDescriptions.emplace_back(ProjectDescription{it->getName(), it->getPath()});
            }
            m_projectCreator.setProjects(std::move(projectDescriptions));
            m_projectCreator.setup([this](ProjectDescription project) {
                createProject(project);
            },[this](ProjectDescription project) {
                deleteProject(project.path);
            },[this](ProjectDescription project) {
                loadProject(project);
            });
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

    // need write to cache file
    void Application::createProject(ProjectDescription projectDescription) {
        std::ifstream ifstream(editorCachePath);
        if(!ifstream.is_open())
            return;
        std::stringstream sstr;
        sstr << ifstream.rdbuf();
        ifstream.close();

        YAML::Node data = YAML::Load(sstr.str());
        YAML::Emitter out;
        if(!data["Robot2D Editor"])
            return;

        YAML::Node projectsNode(YAML::NodeType::Map);
        projectsNode["Path"] = (projectDescription.path);
        projectsNode["Name"] = (projectDescription.name);
        data["Projects"].push_back(projectsNode);

        std::ofstream ofstream(editorCachePath);
        ofstream << data;
        ofstream.close();

        Project::Ptr project = std::make_shared<Project>();
        project -> setPath(projectDescription.path);
        project -> setEditorVersion(editorVersion);
        project -> setName(projectDescription.name);
        project -> setStartScene("Scene.scene");

        std::filesystem::path fullPath(projectDescription.path);
        auto fullname = projectDescription.name + ".robot2D";
        fullPath.append(fullname);
        ProjectSerializer(project).serialize(fullPath.string());
        m_editor.createProject(project);
        m_state = State::Editor;
        m_window.setSize(defaultWindowSize);
    }

    void Application::deleteProject(std::string path) {
        m_editor.deleteProject(path);
        std::ifstream ifstream(editorCachePath);
        if(!ifstream.is_open())
            return;
        std::stringstream sstr;
        sstr << ifstream.rdbuf();
        ifstream.close();

        YAML::Node data = YAML::Load(sstr.str());
        YAML::Emitter out;
        if(!data["Robot2D Editor"] || !data["Projects"])
            return;
        std::vector<ProjectDescription> descriptions;

        for(auto project: data["Projects"]) {
            ProjectDescription description;
            description.path = project["Path"].as<std::string>();
            description.name = project["Name"].as<std::string>();
            if (description.path == path)
                continue;
            descriptions.emplace_back(std::move(description));
        }

        auto version = data["Robot2D Editor"].as<std::string>();
        out << YAML::BeginMap;
        out << YAML::Key << "Robot2D Editor" << YAML::Value << version;
        out << YAML::Key << "Projects" << YAML::Value << YAML::BeginSeq;
        for(const auto& it: descriptions) {
            out << YAML::BeginMap;
            out << YAML::Key << "Path" << YAML::Value << it.path;
            out << YAML::Key << "Name" << YAML::Value << it.name;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream ofstream(editorCachePath);
        ofstream << out.c_str();
        ofstream.close();
    }

    bool Application::parseProjectCache() {
        std::ifstream ifstream(editorCachePath);
        if(!ifstream.is_open())
            return false;
        std::stringstream sstr;
        sstr << ifstream.rdbuf();

        YAML::Node data = YAML::Load(sstr.str());
        if(!data["Robot2D Editor"])
            return false;

        auto projects = data["Projects"];
        if(!projects)
            return false;

        for(auto it: projects) {
            Project::Ptr currentProject = std::make_shared<Project>();
            auto path = it["Path"].as<std::string>();
            auto name = it["Name"].as<std::string>();
            std::filesystem::path fullPath(path);
            auto fullname = name + ".robot2D";
            fullPath.append(fullname);
            if(!ProjectSerializer(currentProject).deserialize(fullPath.string())) {
                RB_EDITOR_ERROR("Can't load {0} from editor cache", fullPath);
                return false;
            }
            m_projectsCache.emplace_back(currentProject);
        }

        return true;
    }

    void Application::loadProject(ProjectDescription project) {
        auto found = std::find_if(m_projectsCache.begin(), m_projectsCache.end(),
                                  [&project](const Project::Ptr it) {
            return (it->getName() == project.name) && (it->getPath() == project.path);
        });
        if(found == m_projectsCache.end()) {
            RB_EDITOR_WARN("Can't load Load project, but have project as disk");
            return;
        }

        m_state = State::Editor;
        m_window.setSize(defaultWindowSize);
        m_editor.loadProject(*found);
    }

}