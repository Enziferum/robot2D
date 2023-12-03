#pragma once

#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Core/MessageBus.hpp>

#include <editor/Scene.hpp>
#include "ICommand.hpp"

namespace editor {
    class DuplicateCommand: public ICommand {
    public:
        DuplicateCommand(robot2D::MessageBus& messageBus,
                         Scene::Ptr activeScene, robot2D::ecs::Entity entity);
        ~DuplicateCommand() = default;

        void undo() override;
        void redo() override;

        DECLARE_CLASS_ID(DuplicateCommand)
    private:
        robot2D::MessageBus& m_messageBus;
        Scene::Ptr m_activeScene{nullptr};
        robot2D::ecs::Entity m_duplicateEntity;
    };
}