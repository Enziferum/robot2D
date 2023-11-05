#pragma once

#include <robot2D/Core/MessageBus.hpp>
#include <robot2D/Ecs/Entity.hpp>

#include "ICommand.hpp"

namespace editor {

    class DeleteChildEntityCommand: public ICommand {
    public:
        DeleteChildEntityCommand(robot2D::MessageBus& messageBus, robot2D::ecs::Entity entity);
        ~DeleteChildEntityCommand() = default;

        void undo() override;

        DECLARE_CLASS_ID(DeleteChildEntityCommand)
    private:
        robot2D::MessageBus& m_messageBus;
        robot2D::ecs::Entity m_entity;
    };

}