/*********************************************************************
(c) Alex Raag 2023
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

#include <robot2D/Util/Logger.hpp>
#include <editor/ApplicationLogic.hpp>
#include <editor/scripting/ScriptingEngine.hpp>

namespace editor {
    ApplicationLogic::ApplicationLogic(MessageDispatcher& messageDispatcher):
    m_messageDispatcher{messageDispatcher},
    m_configuration{},
    m_editorCache{m_configuration},
    m_projectManager{m_configuration},
    m_state{AppState::ProjectInspector}
    {}

    void ApplicationLogic::setup(IEditorOpener* editorModule) {
        auto [hasKey, version] = m_configuration.getValue(ConfigurationKey::Version);
        if(hasKey) {
            RB_EDITOR_WARN("Editor's Current Version {0}.", version);
        }

        m_editorOpener = editorModule;

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
            m_state = AppState::ProjectInspector;
        else
            m_state = AppState::Editor;

        if(m_state == AppState::ProjectInspector) {
            m_messageDispatcher.onMessage<ProjectMessage>(MessageID::CreateProject,
                                                          BIND_CLASS_FN(createProject));
            m_messageDispatcher.onMessage<ProjectMessage>(MessageID::AddProject,
                                                          BIND_CLASS_FN(addProject));
            m_messageDispatcher.onMessage<ProjectMessage>(MessageID::LoadProject,
                                                          BIND_CLASS_FN(loadProject));
            m_messageDispatcher.onMessage<ProjectMessage>(MessageID::DeleteProject,
                                                          BIND_CLASS_FN(deleteProject));

            m_messageDispatcher.onMessage<ShowInspectorMessage>(MessageID::ShowInspector,
                [this](const ShowInspectorMessage& message) {
                    m_editorCache.setShowInspector(message.showAlways);
            });
        } else {
            const auto& projectDescription = m_editorCache.getCurrentProject();
            if(projectDescription.empty())
                RB_EDITOR_WARN("projectDescription.empty()");

            m_messageDispatcher.onMessage<ProjectMessage>(MessageID::CreateProject,
                                                          BIND_CLASS_FN(createProject));
            m_messageDispatcher.onMessage<ProjectMessage>(MessageID::LoadProject,
                                                          BIND_CLASS_FN(loadProject));

            if(!m_projectManager.load(projectDescription)) {
                RB_EDITOR_ERROR("ProjectManager can't load Project := {0}",
                                errorToString(m_projectManager.getError()));
                return;
            }
            auto project = m_projectManager.getCurrentProject();
            m_editorOpener -> loadProject(project);
        }
    }

    void ApplicationLogic::createProject(const ProjectMessage& projectDescription) {
        if(m_projectManager.hasActivateProject()) {
            m_editorOpener -> closeCurrentProject([this, &projectDescription]() {
                createProjectInternal(projectDescription);
            });
        }
        else
            createProjectInternal(projectDescription);
    }

    void ApplicationLogic::createProjectInternal(const ProjectMessage& projectDescription) {
        if(!m_editorCache.addProject(projectDescription.description)) {
            RB_EDITOR_ERROR("Can't add Project to Cache := {0}",
                            errorToString(m_editorCache.getError()));
            return;
        }

        if(!m_projectManager.add(projectDescription.description)) {
            RB_EDITOR_ERROR("Can't create Project := {0}",
                            errorToString(m_projectManager.getError()));
            return;
        }

        auto project = m_projectManager.getCurrentProject();

        std::filesystem::path scriptModulePath{ project -> getPath()};
        scriptModulePath.append("assets\\scripts\\bin");
        scriptModulePath.append(project -> getName());
#ifdef ROBOT2D_WINDOWS
        scriptModulePath += ".dll";
#endif
        /// TODO(a.raag) load somewhere else in real
        ScriptEngine::InitAppRuntime(scriptModulePath);

        m_editorOpener -> createProject(project);
        // m_window -> setResizable(true);
        m_state = AppState::Editor;
    }

    void ApplicationLogic::addProject(const ProjectMessage& projectDescription) {
        if(!m_editorCache.addProject(projectDescription.description)) {
            RB_EDITOR_ERROR("Can't add Project to Cache := {0}",
                            errorToString(m_editorCache.getError()));
            return;
        }

        if(!m_projectManager.load(projectDescription.description)) {
            RB_EDITOR_ERROR("Can't create Project := {0}",
                            errorToString(m_projectManager.getError()));
            return;
        }

        auto project = m_projectManager.getCurrentProject();

        std::filesystem::path scriptModulePath{ project -> getPath()};
        scriptModulePath.append("assets\\scripts\\bin");
        scriptModulePath.append(project -> getName());
#ifdef ROBOT2D_WINDOWS
        scriptModulePath += ".dll";
#endif
        /// TODO(a.raag) load somewhere else in real
        ScriptEngine::InitAppRuntime(scriptModulePath);

        m_editorOpener -> loadProject(project);
        // m_window -> setResizable(true);
        m_state = AppState::Editor;
    }


    void ApplicationLogic::loadProject(const ProjectMessage& projectDescription) {
        if(m_projectManager.hasActivateProject()) {
            m_editorOpener -> closeCurrentProject([this, &projectDescription]() {
                loadProjectInternal(projectDescription);
            });
        }
        else
            loadProjectInternal(projectDescription);
    }

    void ApplicationLogic::loadProjectInternal(const ProjectMessage& projectDescription) {
        if(!m_editorCache.loadProject(projectDescription.description)) {
            RB_EDITOR_ERROR("Cache can't load Project := {0}",
                            errorToString(m_editorCache.getError()));
            return;
        }

        if(!m_projectManager.load(projectDescription.description)) {
            RB_EDITOR_ERROR("ProjectManager can't load Project := {0}",
                            errorToString(m_projectManager.getError()));
            return;
        }

        m_state = AppState::Editor;
        // m_window -> setResizable(true);
        auto project = m_projectManager.getCurrentProject();

        std::filesystem::path scriptModulePath{ project -> getPath()};
        scriptModulePath.append("assets\\scripts\\bin");
        scriptModulePath.append(project -> getName());
#ifdef ROBOT2D_WINDOWS
        scriptModulePath += ".dll";
#endif
        if(exists(scriptModulePath))
            ScriptEngine::InitAppRuntime(scriptModulePath);

        m_editorOpener -> loadProject(project);
    }


    void ApplicationLogic::deleteProject(const ProjectMessage& projectDescription) {

        if(!m_editorCache.removeProject(projectDescription.description)) {
            RB_EDITOR_ERROR("EditorCache can't delete Project := {0}",
                            errorToString(m_editorCache.getError()));
            return;
        }

        if(!m_projectManager.remove(projectDescription.description)) {
            RB_EDITOR_ERROR("ProjectManager can't delete Project := {0}",
                            errorToString(m_projectManager.getError()));
            return;
        }
    }



} // namespace editor


