#pragma once
#include <vector>
#include <robot2D/Ecs/Entity.hpp>
#include <editor/panels/ITreeItem.hpp>

#include "EditorState.hpp"
#include "DeletedEntitesRestoreInformation.hpp"
#include "PopupConfiguration.hpp"

namespace editor {

    class IEditor;
    class EditorPresenter {
    public:
        EditorPresenter(IEditor* editor);
        ~EditorPresenter() = default;

        void findSelectedEntitiesOnUI(std::vector<ITreeItem::Ptr>&& selectedEntities);
        void clearSelectionOnUI();

        void prepareView();
        void switchState(EditorState editorState) { m_state = editorState; }
        const EditorState& getState() const;

        void showPopup(PopupConfiguration* popupConfiguration);

        DeletedEntitiesRestoreUIInformation removeEntitiesFromUI(std::vector<ITreeItem::Ptr>&& uiItems);
        void restoreEntitiesOnUI(DeletedEntitiesRestoreUIInformation& restoreUiInformation);
        void setMainCameraEntity(robot2D::ecs::Entity entity);
    private:
        IEditor* m_editor;
        EditorState m_state{EditorState::Edit};
    };

} // namespace editor