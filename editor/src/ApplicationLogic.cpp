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

    void ApplicationLogic::setup(EditorLogic* editorLogic) {
        m_editorLogic = editorLogic;
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

            if(!m_projectManager.load(projectDescription)) {
                RB_EDITOR_ERROR("ProjectManager can't load Project := {0}",
                                errorToString(m_projectManager.getError()));
                return;
            }
            auto project = m_projectManager.getCurrentProject();
            m_editorLogic -> loadProject(project);
        }
    }

    void ApplicationLogic::createProject(const ProjectMessage& projectDescription) {
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
        scriptModulePath += ".dll";

        /// TODO(a.raag) load somewhere else in real
        ScriptEngine::InitAppRuntime(scriptModulePath);

        m_editorLogic -> createProject(project);
        // m_window -> setResizable(true);
        m_state = AppState::Editor;
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

    void ApplicationLogic::loadProject(const ProjectMessage& projectDescription) {
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
        scriptModulePath += ".dll";

        if(exists(scriptModulePath))
            ScriptEngine::InitAppRuntime(scriptModulePath);

        m_editorLogic -> loadProject(project);
    }

} // namespace editor


