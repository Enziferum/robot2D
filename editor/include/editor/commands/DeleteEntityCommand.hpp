#pragma once

#include <robot2D/Core/MessageBus.hpp>
#include <robot2D/Ecs/Entity.hpp>

#include "ICommand.hpp"

namespace editor {
    class DeleteEntityCommand: public ICommand {
    public:
        DeleteEntityCommand(robot2D::MessageBus& messageBus, robot2D::ecs::Entity entity);
        ~DeleteEntityCommand() override = default;

        void undo() override;

        DECLARE_CLASS_ID(DeleteEntityCommand)
    private:
        robot2D::MessageBus& m_messageBus;
        robot2D::ecs::Entity m_entity;
    };
}