#pragma once
#include <vector>
#include <list>
#include <string>
#include <functional>
#include <robot2D/Ecs/Entity.hpp>

#include "Uuid.hpp"
#include "DeletedEntitesRestoreInformation.hpp"
#include "CommandStack.hpp"
#include "ExportOptions.hpp"

namespace editor {

    class UIInteractor {
    public:
        using Ptr = UIInteractor*;

        virtual ~UIInteractor() = 0;
        virtual std::vector<robot2D::ecs::Entity>& getSelectedEntities()  = 0;
        virtual robot2D::ecs::Entity getSelectedEntity(int graphicsEntityID) = 0;
        virtual void restoreDeletedEntities(DeletedEntitiesRestoreInformation& restoreInformation,
                                            DeletedEntitiesRestoreUIInformation& restoreUiInformation) = 0;


        virtual std::string getAssociatedProjectPath() const = 0;
        virtual std::list<robot2D::ecs::Entity> getEntities() const = 0;
        virtual void removeEntity(robot2D::ecs::Entity entity) = 0;
        virtual void addEmptyEntity() = 0;
        virtual robot2D::ecs::Entity createEmptyEntity() = 0;
        virtual robot2D::ecs::Entity duplicateEmptyEntity(robot2D::ecs::Entity entity) = 0;
        virtual void setBefore(robot2D::ecs::Entity sourceEntity, robot2D::ecs::Entity target) = 0;
        virtual void removeEntityChild(robot2D::ecs::Entity entity) = 0;
        virtual bool isRunning() const = 0;
        virtual robot2D::ecs::Entity getByUUID(UUID uuid) = 0;
        virtual void registerOnDeleteFinish(std::function<void()>&& callback) = 0;
        virtual void uiSelectedAllEntities() = 0;
        virtual void uiSelectedRangeEntities(std::vector<robot2D::ecs::Entity>&& entities) = 0;

        virtual void exportProject(const ExportOptions& exportOptions) = 0;
        virtual const std::vector<class_id>& getCommandStack() const = 0;
    };

} // namespace editor