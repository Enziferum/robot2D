#pragma once
#include <vector>
#include <robot2D/Core/MessageBus.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include "ICommand.hpp"

namespace editor {

    class DeleteEntitiesCommand: public ICommand {
    public:
        DeleteEntitiesCommand(robot2D::MessageBus& messageBus, std::vector<robot2D::ecs::Entity> entities);
        ~DeleteEntitiesCommand() = default;

        DECLARE_CLASS_ID(DeleteEntitiesCommand)

        void undo() override;
    private:
        robot2D::MessageBus& m_messageBus;
        std::vector<robot2D::ecs::Entity> m_entities;
    };

} // namespace editor