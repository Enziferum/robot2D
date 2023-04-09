#pragma once
#include "EditorState.hpp"

namespace editor {

    class IEditor;
    class EditorPresenter {
    public:
        EditorPresenter(IEditor* editor);
        ~EditorPresenter() = default;

        void prepareView();
        void switchState(EditorState editorState) { m_state = editorState; }
        const EditorState& getState() const;
    private:
        IEditor* m_editor;
        EditorState m_state{EditorState::Edit};
    };

} // namespace editor