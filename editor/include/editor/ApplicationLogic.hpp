#pragma once

#include "Configuration.hpp"
#include "EditorCache.hpp"
#include "MessageDispather.hpp"
#include "Messages.hpp"
#include "ProjectManager.hpp"
#include "EditorLogic.hpp"

namespace editor {
    enum class AppState {
        ProjectInspector,
        Editor
    };

    class ApplicationLogic {
    public:
        ApplicationLogic(MessageDispatcher& messageDispatcher);
        ~ApplicationLogic() = default;

        void setup(EditorLogic* editorLogic);
        const AppState& getState() const { return m_state; }
        const EditorCache& getCache() const { return m_editorCache; }
    private:
        void createProject(const ProjectMessage& projectDescription);
        void deleteProject(const ProjectMessage& projectDescription);
        void loadProject(const ProjectMessage& projectDescription);
    private:
        MessageDispatcher& m_messageDispatcher;
        Configuration m_configuration;
        EditorCache m_editorCache;
        ProjectManager m_projectManager;
        AppState m_state;
        EditorLogic* m_editorLogic;
    };
}