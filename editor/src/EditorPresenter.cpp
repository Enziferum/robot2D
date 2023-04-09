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
} // namespace editor