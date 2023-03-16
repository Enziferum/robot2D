#include <editor/EditorLogic.hpp>
#include <editor/Editor.hpp>

namespace editor {

    class SceneLoadTask: public ITask {
    public:
        SceneLoadTask(ITaskFunction::Ptr function,
                      SceneManager& sceneManager,
                      Project::Ptr project):
                ITask{std::move(function)},
                m_sceneManager{sceneManager},
                m_project{std::move(project)}{}
        ~SceneLoadTask() override = default;

        void execute() override {
            if(!m_sceneManager.load(std::move(m_project))) {
                RB_EDITOR_ERROR("Can't Load Scene. Reason: {0}",
                                errorToString(m_sceneManager.getError()));
                return;
            }
        }
    private:
        SceneManager& m_sceneManager;
        Project::Ptr m_project;
    };


    EditorLogic::EditorLogic(robot2D::MessageBus& messageBus):
    m_messageBus{messageBus},
    m_sceneManager{messageBus},
    m_activeScene{nullptr}
    {}

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
        //m_state = State::Load;
//
//        auto loadLambda = [this](const SceneLoadTask& task) {
//            m_activeScene = m_sceneManager.getActiveScene();
//            m_window -> setMaximazed(true);
//
//            prepare();
//            openScene();
//            m_state = State::Edit;
//        };
//
//        m_taskQueue.addAsyncTask<SceneLoadTask>(loadLambda,
//                                              m_sceneManager, project);

        if(!m_sceneManager.load(std::move(project))) {
            RB_EDITOR_ERROR("Can't Load Scene. Reason: {0}",
                            errorToString(m_sceneManager.getError()));
            return;
        }

        m_activeScene = m_sceneManager.getActiveScene();
        m_activeScene = m_sceneManager.getActiveScene();
        m_editor -> openScene(m_activeScene, m_currentProject -> getPath());
    }

    bool EditorLogic::saveScene() {
        return m_sceneManager.save(std::move(m_activeScene));
    }

} // namespace editor



