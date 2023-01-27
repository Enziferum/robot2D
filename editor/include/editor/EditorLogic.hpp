#pragma once

#include "Project.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"

namespace editor {
    class IEditor;
    class EditorLogic {
    public:
        EditorLogic(robot2D::MessageBus& messageBus);
        ~EditorLogic() = default;

        void setIEditor(IEditor* iEditor) { m_editor = iEditor; }
        bool saveScene();

        void createProject(Project::Ptr project);
        void loadProject(Project::Ptr project);
    private:
        robot2D::MessageBus& m_messageBus;
        Project::Ptr m_currentProject;
        SceneManager m_sceneManager;
        Scene::Ptr m_activeScene;
        IEditor* m_editor{nullptr};
    };
}