#include <editor/EditorPresenter.hpp>
#include <editor/Editor.hpp>

namespace editor {

    EditorPresenter::EditorPresenter(IEditor* editor): m_editor{editor} {}

    const EditorState& EditorPresenter::getState() const {
        return m_state;
    }

    void EditorPresenter::prepareView() {
        m_editor -> prepare();
    }

    void EditorPresenter::restoreEntitiesOnUI(DeletedEntitiesRestoreUIInformation &restoreUiInformation) {
        m_editor -> restoreEntitiesOnUI(restoreUiInformation);
    }

    DeletedEntitiesRestoreUIInformation
    EditorPresenter::removeEntitiesFromUI(std::vector<robot2D::ecs::Entity> removingEntities) {
        return m_editor -> removeEntitiesOnUI(removingEntities);
    }

    void EditorPresenter::findSelectedEntitiesOnUI(std::vector<robot2D::ecs::Entity>& selectedEntities) {
        m_editor -> findSelectedEntitiesOnUI(selectedEntities);
    }
} // namespace editor