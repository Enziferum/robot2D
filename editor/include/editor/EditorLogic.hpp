#pragma once

#include "Project.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"
#include "MessageDispather.hpp"
#include "Messages.hpp"
#include "TaskQueue.hpp"

namespace editor {
    class IEditor;
    class EditorLogic {
    public:
        enum class State {
            Load,
            Edit,
            Run
        };
    public:
        EditorLogic(robot2D::MessageBus& messageBus, MessageDispatcher& messageDispatcher);
        EditorLogic(const EditorLogic& other) = delete;
        EditorLogic& operator=(const EditorLogic& other) = delete;
        EditorLogic(EditorLogic&& other) = delete;
        EditorLogic& operator=(EditorLogic&& other) = delete;
        ~EditorLogic() = default;

        void setIEditor(IEditor* iEditor) { m_editor = iEditor; }
        bool saveScene();

        void createScene();
        void copyToBuffer();
        void pasterFromBuffer();

        void createProject(Project::Ptr project);
        void loadProject(Project::Ptr project);

        void setState(State state) { m_state = state;}
        State getState() {  return m_state; }
    private:
        void loadCallback();
        void saveScene(const MenuProjectMessage& message);
        void toolbarPressed(const ToolbarMessage& message);
    private:
        robot2D::MessageBus& m_messageBus;
        MessageDispatcher& m_messageDispatcher;
        Project::Ptr m_currentProject;
        SceneManager m_sceneManager;
        Scene::Ptr m_activeScene;
        IEditor* m_editor{nullptr};
        State m_state{State::Run};
        int val = 12;
    };
}