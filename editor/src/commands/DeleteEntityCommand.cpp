#include <editor/commands/DeleteEntityCommand.hpp>

namespace editor {

    DeleteEntityCommand::DeleteEntityCommand(robot2D::MessageBus& messageBus, robot2D::ecs::Entity entity):
    m_messageBus{messageBus},
    m_entity{entity}
    {
    }

    const class_id& DeleteEntityCommand::id() noexcept {
        static const class_id id{"DeleteEntityCommand"};
        return id;
    }


    void DeleteEntityCommand::undo() {

    }

} // namespace editor