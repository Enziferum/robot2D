#include <editor/EditorRouter.hpp>
#include <editor/Editor.hpp>

namespace editor {
    EditorRouter::EditorRouter(editor::IEditor* editorView):
    m_editorView{editorView} {}

    bool EditorRouter::openScene(Scene::Ptr scene, std::string path) {
        m_editorView -> openScene(scene, path);
        return true;
    }

} // namespace editor