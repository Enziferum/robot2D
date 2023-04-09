#pragma once
#include <string>
#include "Scene.hpp"


namespace editor {

    class IEditor;
    class EditorRouter {
    public:
        EditorRouter(IEditor* editorView);
        ~EditorRouter() = default;

        bool openScene(Scene::Ptr scene, std::string path);
    private:
        IEditor* m_editorView{nullptr};
    };

} // namespace editor