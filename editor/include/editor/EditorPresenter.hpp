#pragma once
#include <vector>
#include <robot2D/Ecs/Entity.hpp>

#include "EditorState.hpp"
#include "DeletedEntitesRestoreInformation.hpp"

namespace editor {

    class IEditor;
    class EditorPresenter {
    public:
        EditorPresenter(IEditor* editor);
        ~EditorPresenter() = default;

        void findSelectedEntitiesOnUI(std::vector<robot2D::ecs::Entity>& selectedEntities);
        void prepareView();
        void switchState(EditorState editorState) { m_state = editorState; }
        const EditorState& getState() const;

        DeletedEntitiesRestoreUIInformation removeEntitiesFromUI(std::vector<robot2D::ecs::Entity> removingEntities);
        void restoreEntitiesOnUI(DeletedEntitiesRestoreUIInformation& restoreUiInformation);
    private:
        IEditor* m_editor;
        EditorState m_state{EditorState::Edit};
    };

} // namespace editor