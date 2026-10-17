/*********************************************************************
   (c) Alex Raag 2026
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
#include <editor/scripting/ScriptProject.hpp>
#include <editor/scripting/ScriptingEngine.hpp>
#include <editor/Exception.hpp>
#include <editor/BuildInfo.hpp>

namespace editor {
    ApplicationLogic::ApplicationLogic(MessageDispatcher& messageDispatcher) :
        m_messageDispatcher{messageDispatcher},
        m_configuration{},
        m_editorCache{m_configuration},
        m_projectManager{m_configuration},
        m_state{AppState::ProjectInspector} {
    }

    void ApplicationLogic::setup(IEditorOpener* editorModule, ScriptingEngineService::Ptr scriptingEngine) {
        RB_EDITOR_INFO("Editor's Current Version {0}, build {1}.",
            buildInfo::getVersion(), buildInfo::buildID);

        m_editorOpener = editorModule;
        m_scriptingEngineService = scriptingEngine;

        auto [status, result] = m_configuration.getValue(ConfigurationKey::CachePath);

        if(!m_editorCache.parseCache(result)) {
            RB_EDITOR_WARN("Editor cache: {0}. Opening project browser.",
                errorToString(m_editorCache.getError()));
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
            m_messageDispatcher.onMessage<ProjectMessage>(MessageID::RemoveRecentProject,
                [this](const ProjectMessage& message) {
                if(!m_editorCache.removeProject(message.description)) {
                    RB_EDITOR_ERROR("Can't remove project from recent list: {0}",
                    errorToString(m_editorCache.getError()));
                    RB2D_EXCEPTION("Editor Cache Error");
                }
            });

            m_messageDispatcher.onMessage<ShowInspectorMessage>(MessageID::ShowInspector,
                [this](const ShowInspectorMessage& message) {
                if(!m_editorCache.setShowInspector(message.showAlways))
                    RB_EDITOR_ERROR("Can't save project browser startup preference");
            });
        }
        else {
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
                RB2D_EXCEPTION("Project loading");
            }
            auto project = m_projectManager.getCurrentProject();
            auto built = buildProjectScripts(project -> getPath(), project -> getName(), scriptProjectTools());
            if(!built.success) {
                m_scriptingEngineService -> runtimeInit({});
                RB_EDITOR_ERROR("[ScriptBuild] Startup project scripts unavailable; project='{}'; reason={}",
                    project -> getPath(), built.diagnostic);
            }
            else if(!m_scriptingEngineService -> runtimeInit(built.assembly.u8string()))
                RB_EDITOR_ERROR("[ScriptBuild] Assembly load failed; path='{}'", built.assembly.u8string());
            m_editorOpener -> loadProject(project);
        }
    }

    void ApplicationLogic::createProject(const ProjectMessage& projectDescription) {
        if(m_projectManager.hasActivateProject()) {
            m_editorOpener -> closeCurrentProject([this, projectDescription]() {
                createProjectInternal(projectDescription);
            });
        }
        else
            createProjectInternal(projectDescription);
    }

    void ApplicationLogic::createProjectInternal(const ProjectMessage& projectDescription) {
        if(!m_editorCache.addProject(projectDescription.description)) {
            RB_EDITOR_ERROR("Can't create project to Cache := {0}",
                errorToString(m_editorCache.getError()));
            RB2D_EXCEPTION("Editor Cache Error");
        }

        if(!m_projectManager.add(projectDescription.description)) {
            RB_EDITOR_ERROR("Can't create project := {0}",
                errorToString(m_projectManager.getError()));
            RB2D_EXCEPTION("ProjectManager Error");
        }

        auto project = m_projectManager.getCurrentProject();
        auto built = buildProjectScripts(project -> getPath(), project -> getName(), scriptProjectTools());
        if(!built.success) {
            m_scriptingEngineService -> runtimeInit({});
            RB_EDITOR_ERROR("[ScriptBuild] New project script build failed; project='{}'; reason={}",
                project -> getPath(), built.diagnostic);
        }
        else if(!m_scriptingEngineService -> runtimeInit(built.assembly.u8string()))
            RB_EDITOR_ERROR("[ScriptBuild] Assembly load failed; path='{}'", built.assembly.u8string());
        m_editorOpener -> createProject(project);
        m_state = AppState::Editor;
    }

    void ApplicationLogic::addProject(const ProjectMessage& projectDescription) {
        if(!m_editorCache.addProject(projectDescription.description)) {
            RB_EDITOR_ERROR("Can't add Project to Cache := {0}",
                errorToString(m_editorCache.getError()));
            RB2D_EXCEPTION("Editor Cache Error");
        }

        if(!m_projectManager.load(projectDescription.description)) {
            RB_EDITOR_ERROR("Can't load Project := {0}",
                errorToString(m_projectManager.getError()));
            RB2D_EXCEPTION("ProjectManager Error");
        }

        auto project = m_projectManager.getCurrentProject();

        auto built = buildProjectScripts(project -> getPath(), project -> getName(), scriptProjectTools());
        if(!built.success) {
            m_scriptingEngineService -> runtimeInit({});
            RB_EDITOR_ERROR("[ScriptBuild] Project scripts unavailable; project='{}'; reason={}", project -> getPath(),
                built.diagnostic);
        }
        else if(!m_scriptingEngineService -> runtimeInit(built.assembly.u8string()))
            RB_EDITOR_ERROR("[ScriptBuild] Assembly load failed; path='{}'", built.assembly.u8string());

        m_editorOpener -> loadProject(project);
        m_state = AppState::Editor;
    }


    void ApplicationLogic::loadProject(const ProjectMessage& projectDescription) {
        if(m_projectManager.hasActivateProject()) {
            m_editorOpener -> closeCurrentProject([this, projectDescription]() {
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
            RB2D_EXCEPTION("Editor Cache Error");
        }

        if(!m_projectManager.load(projectDescription.description)) {
            RB_EDITOR_ERROR("ProjectManager can't load Project := {0}",
                errorToString(m_projectManager.getError()));
            RB2D_EXCEPTION("ProjectManager Error");
        }

        m_state = AppState::Editor;
        auto project = m_projectManager.getCurrentProject();

        auto built = buildProjectScripts(project -> getPath(), project -> getName(), scriptProjectTools());
        if(!built.success) {
            m_scriptingEngineService -> runtimeInit({});
            RB_EDITOR_ERROR("[ScriptBuild] Project scripts unavailable; project='{}'; reason={}", project -> getPath(),
                built.diagnostic);
        }
        else if(!m_scriptingEngineService -> runtimeInit(built.assembly.u8string()))
            RB_EDITOR_ERROR("[ScriptBuild] Assembly load failed; path='{}'", built.assembly.u8string());

        m_editorOpener -> loadProject(project);
    }


    void ApplicationLogic::deleteProject(const ProjectMessage& projectDescription) {

        if(!m_editorCache.removeProject(projectDescription.description)) {
            RB_EDITOR_ERROR("EditorCache can't delete Project := {0}",
                errorToString(m_editorCache.getError()));
            RB2D_EXCEPTION("Editor Cache Error");
        }

        if(!m_projectManager.remove(projectDescription.description)) {
            RB_EDITOR_ERROR("ProjectManager can't delete Project := {0}",
                errorToString(m_projectManager.getError()));
            RB2D_EXCEPTION("ProjectManager Error");
        }
    }

} // namespace editor


