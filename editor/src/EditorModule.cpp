#include <editor/EditorModule.hpp>

namespace editor {

    IEditorOpener::~IEditorOpener() = default;
    IEditorModule::~IEditorModule() = default;

    EditorModule::EditorModule(robot2D::MessageBus& messageBus,
                               MessageDispatcher& messageDispatcher, robot2D::Gui& gui)
    : m_editor{messageBus, gui},
      m_router{&m_editor},
      m_presenter{&m_editor},
      m_interactor{messageBus, messageDispatcher, m_presenter, m_router}
    {}

    void EditorModule::setup(robot2D::RenderWindow* window) {
        m_editor.setup(window, &m_interactor);
        m_interactor.setView(&m_editor);
    }

    void EditorModule::handleEvents(const robot2D::Event& event) {
        m_editor.handleEvents(event);
    }

    void EditorModule::handleMessages(const robot2D::Message &message) {
        m_editor.handleMessages(message);
    }

    void EditorModule::update(float dt) {
        m_editor.update(dt);
    }

    void EditorModule::render() {
        m_editor.render();
    }


    void EditorModule::createProject(Project::Ptr project) {
        m_interactor.createProject(project);
    }

    void EditorModule::loadProject(Project::Ptr project) {
        m_interactor.loadProject(project);
    }

    void EditorModule::destroy() {
        m_interactor.destroy();
    }

    void EditorModule::closeCurrentProject(std::function<void()>&& resultCallback) {
        m_interactor.closeCurrentProject(std::move(resultCallback));
    }

} // namespace editor

