#pragma once
#include <memory>
#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Core/Message.hpp>
#include <robot2D/Core/MessageBus.hpp>
#include <robot2D/imgui/Gui.hpp>

#include "MessageDispather.hpp"
#include "Project.hpp"
#include "EditorModule.hpp"

namespace editor {


    class EditorAssembly {
    public:
        static IEditorModule::Ptr createEditorModule(robot2D::RenderWindow* renderWindow,
                                                     robot2D::MessageBus& messageBus,
                                                     MessageDispatcher& messageDispatcher,
                                                     robot2D::Gui& gui);
    };

} // namespace editor