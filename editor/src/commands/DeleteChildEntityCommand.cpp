#include <editor/commands/DeleteChildEntityCommand.hpp>

namespace editor {

    DeleteChildEntityCommand::DeleteChildEntityCommand(robot2D::MessageBus& messageBus,
                                                        robot2D::ecs::Entity entity):
                                                        m_messageBus{messageBus},
                                                        m_entity(entity)
    {
    }

    void DeleteChildEntityCommand::undo() {

    }
} // namespace editor