#pragma once
#include <list>
#include <vector>

#include <editor/SceneEntity.hpp>

#include "EditorState.hpp"
#include "DeletedEntitesRestoreInformation.hpp"
#include "PopupConfiguration.hpp"

namespace editor {

    class IEditor;
    class EditorPresenter {
    public:
        EditorPresenter(IEditor* editor);
        EditorPresenter(const EditorPresenter& other) = delete;
        EditorPresenter& operator=(const EditorPresenter& other) = delete;
        EditorPresenter(EditorPresenter&& other) = delete;
        EditorPresenter& operator=(EditorPresenter&& other) = delete;
        ~EditorPresenter() = default;

        void clearSelectionOnUI();

        void prepareView();
        void switchState(EditorState editorState) { m_state = editorState; }
        const EditorState& getState() const;

        void showPopup(PopupConfiguration* popupConfiguration);

        void setMainCameraEntity(SceneEntity entity);
    private:
        IEditor* m_editor;
        EditorState m_state{EditorState::Edit};
    };

} // namespace editor
