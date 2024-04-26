#pragma once

#include <vector>
#include <robot2D/Core/MessageBus.hpp>

#include <editor/SceneEntity.hpp>
#include <editor/Scene.hpp>
#include "ICommand.hpp"

namespace editor {
    class DuplicateCommand: public ICommand {
    public:
        DuplicateCommand(robot2D::MessageBus& messageBus,
                         Scene::Ptr activeScene, std::vector<SceneEntity> entities);
        DuplicateCommand(const DuplicateCommand& other) = delete;
        DuplicateCommand& operator=(const DuplicateCommand& other) = delete;
        DuplicateCommand(DuplicateCommand&& other) = delete;
        DuplicateCommand& operator=(DuplicateCommand&& other) = delete;
        ~DuplicateCommand() = default;

        void undo() override;
        void redo() override;

        DECLARE_CLASS_ID(DuplicateCommand)
    private:
        robot2D::MessageBus& m_messageBus;
        Scene::Ptr m_activeScene{nullptr};
        std::vector<SceneEntity> m_entities;
    };
}