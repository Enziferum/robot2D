#include <editor/EditorLogic.hpp>
#include <editor/Editor.hpp>
#include <editor/FileApi.hpp>
#include <editor/Messages.hpp>

namespace {
    const std::string scenePath = "assets/scenes";
}

namespace editor {

    class SceneLoadTask: public ITask {
    public:
        SceneLoadTask(ITaskFunction::Ptr function,
                      SceneManager& sceneManager,
                      Project::Ptr project,
                      std::string path,
                      EditorLogic* l):
                ITask{function},
                m_sceneManager{sceneManager},
                m_project{std::move(project)},
                m_path{path},
                logic{l}{}
        ~SceneLoadTask() override = default;

        void execute() override {
            if(!m_sceneManager.load(m_project, m_path)) {
                RB_EDITOR_ERROR("Can't Load Scene. Reason: {0}",
                                errorToString(m_sceneManager.getError()));
                return;
            }
        }

    private:
        SceneManager& m_sceneManager;
        Project::Ptr m_project;
        std::string m_path;
    public:
        EditorLogic* logic;
    };


    EditorLogic::EditorLogic(robot2D::MessageBus& messageBus,
                             MessageDispatcher& messageDispatcher,
                             TaskQueue& taskQueue):
    m_messageBus{messageBus},
    m_messageDispatcher{messageDispatcher},
    m_taskQueue{taskQueue},
    m_sceneManager{messageBus},
    m_activeScene{nullptr}
    {
        m_messageDispatcher.onMessage<MenuProjectMessage>(MessageID::SaveScene, BIND_CLASS_FN(saveScene));
    }

    void EditorLogic::createProject(Project::Ptr project) {
        m_currentProject = project;
        if(!m_sceneManager.add(std::move(project))) {
            RB_EDITOR_ERROR("Can't Create Scene. Reason: {0}",
                            errorToString(m_sceneManager.getError()));
            return;
        }
        m_activeScene = m_sceneManager.getActiveScene();
        m_editor -> openScene(m_activeScene, m_currentProject -> getPath());
    }

    void EditorLogic::loadProject(Project::Ptr project) {
        m_currentProject = project;
        m_state = State::Load;

        auto loadLambda = [](const SceneLoadTask& task) {
            task.logic -> loadCallback();
        };

        auto path = project -> getPath();
        auto appendPath = combinePath(scenePath, project -> getStartScene());
        auto scenePath1 = combinePath(path, appendPath);

        m_editor -> prepare();
        m_taskQueue.addAsyncTask<SceneLoadTask>(loadLambda, m_sceneManager, project, scenePath1, this);
    }

    bool EditorLogic::saveScene() {
        return m_sceneManager.save(std::move(m_activeScene));
    }

    void EditorLogic::loadCallback() {
        m_state = State::Edit;
        m_activeScene = m_sceneManager.getActiveScene();
        m_editor -> openScene(m_activeScene, m_currentProject -> getPath());
    }

    void EditorLogic::saveScene(const MenuProjectMessage& message) {
        m_sceneManager.save(std::move(m_activeScene));
    }


    void EditorLogic::copyToBuffer() {
        /// find selections ///
    }

    void EditorLogic::pasterFromBuffer() {

    }

} // namespace editor


