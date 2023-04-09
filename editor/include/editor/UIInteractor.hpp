#pragma once
#include <vector>
#include <string>
#include <functional>
#include <robot2D/Ecs/Entity.hpp>
#include "Uuid.hpp"

namespace editor {

    class UIInteractor {
    public:
        using Ptr = UIInteractor*;

        virtual ~UIInteractor() = 0;
        virtual std::vector<robot2D::ecs::Entity> getSelectedEntities() const = 0;

        virtual std::string getAssociatedProjectPath() const = 0;
        virtual std::vector<robot2D::ecs::Entity> getEntities() const = 0;
        virtual void removeEntity(robot2D::ecs::Entity entity) = 0;

        virtual void addEmptyEntity() = 0;
        virtual robot2D::ecs::Entity createEmptyEntity() = 0;
        virtual robot2D::ecs::Entity duplicateEmptyEntity(robot2D::ecs::Entity entity) = 0;
        virtual void setBefore(robot2D::ecs::Entity sourceEntity, robot2D::ecs::Entity target) = 0;
        virtual void removeEntityChild(robot2D::ecs::Entity entity) = 0;
        virtual bool isRunning() const = 0;
        virtual robot2D::ecs::Entity getByUUID(UUID uuid) = 0;
        virtual void registerOnDeleteFinish(std::function<void()>&& callback) = 0;
    };

} // namespace editor