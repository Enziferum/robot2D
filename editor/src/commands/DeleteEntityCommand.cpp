#include <editor/commands/DeleteEntityCommand.hpp>

namespace editor {

    DeleteEntityCommand::DeleteEntityCommand(robot2D::MessageBus& messageBus, robot2D::ecs::Entity entity):
    m_messageBus{messageBus},
    m_entity{entity}
    {
    }

    void DeleteEntityCommand::undo() {

    }

} // namespace editor