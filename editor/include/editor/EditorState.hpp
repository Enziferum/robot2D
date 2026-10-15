#pragma once

namespace editor {

    enum class EditorState {
        Load,
        Edit,
        Run,
        Modal,
        LoadFailed
    };

} // namespace editor
