#include <editor/EditorPresenter.hpp>
#include <editor/Editor.hpp>

namespace editor {

    EditorPresenter::EditorPresenter(IEditor* editor): m_editor{editor} {}

    const EditorState& EditorPresenter::getState() const {
        return m_state;
    }

    void EditorPresenter::prepareView() {
        m_editor -> prepareView();
    }

    void EditorPresenter::restoreEntitiesOnUI(DeletedEntitiesRestoreUIInformation &restoreUiInformation) {
        m_editor -> restoreEntitiesOnUI(restoreUiInformation);
    }

    DeletedEntitiesRestoreUIInformation
    EditorPresenter::removeEntitiesFromUI(std::list<ITreeItem::Ptr>&& uiItems) {
        return m_editor -> removeEntitiesOnUI(std::move(uiItems));
    }

    void EditorPresenter::findSelectedEntitiesOnUI(std::vector<ITreeItem::Ptr>&& items) {
        m_editor -> findSelectedEntitiesOnUI(std::move(items));
    }

    void EditorPresenter::clearSelectionOnUI() {
        m_editor -> clearSelectionOnUI();
    }

    void EditorPresenter::showPopup(PopupConfiguration* popupConfiguration) {
        m_editor -> showPopup(popupConfiguration);
    }

    void EditorPresenter::setMainCameraEntity(SceneEntity entity) {
        m_editor -> setMainCameraEntity(entity);
    }
} // namespace editor