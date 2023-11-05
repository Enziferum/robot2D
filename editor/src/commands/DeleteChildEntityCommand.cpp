#include <editor/commands/DeleteChildEntityCommand.hpp>

namespace editor {

    DeleteChildEntityCommand::DeleteChildEntityCommand(robot2D::MessageBus& messageBus,
                                                        robot2D::ecs::Entity entity):
                                                        m_messageBus{messageBus},
                                                        m_entity(entity)
    {
    }

    const class_id& DeleteChildEntityCommand::id() noexcept {
        static const class_id id{"DeleteChildEntityCommand"};
        return id;
    }

    void DeleteChildEntityCommand::undo() {

    }
} // namespace editor