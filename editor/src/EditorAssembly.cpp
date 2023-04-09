#include <editor/EditorAssembly.hpp>

#include <editor/Editor.hpp>
#include <editor/EditorLogic.hpp>
#include <editor/EditorPresenter.hpp>
#include <editor/EditorRouter.hpp>

namespace editor {

    IEditorModule::Ptr EditorAssembly::createEditorModule(robot2D::RenderWindow* renderWindow,
                                                          robot2D::MessageBus& messageBus,
                                                          MessageDispatcher& messageDispatcher,
                                                          robot2D::Gui& gui) {
        EditorModule::Ptr editorModule = std::make_unique<EditorModule>(messageBus, messageDispatcher, gui);
        return editorModule;
    }

} // namespace editor