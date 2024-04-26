#pragma once
#include <vector>
#include <list>
#include <string>
#include <set>
#include <functional>
#include <editor/SceneEntity.hpp>

#include <editor/panels/ITreeItem.hpp>
#include "Uuid.hpp"
#include "DeletedEntitesRestoreInformation.hpp"
#include "CommandStack.hpp"
#include "ExportOptions.hpp"

namespace editor {

    class UIInteractor {
    public:
        using Ptr = UIInteractor*;

        virtual ~UIInteractor() = 0;

        virtual std::vector<SceneEntity>& getSelectedEntities()  = 0;
        virtual SceneEntity getSelectedEntity(int graphicsEntityID) = 0;

        virtual void restoreDeletedEntities(DeletedEntitiesRestoreInformation& restoreInformation,
                                            DeletedEntitiesRestoreUIInformation& restoreUiInformation) = 0;


        virtual std::string getAssociatedProjectPath() const = 0;
        virtual std::list<SceneEntity> getEntities() const = 0;
        virtual void removeEntity(SceneEntity entity) = 0;
        virtual void addEmptyEntity() = 0;
        virtual SceneEntity addButton() = 0;
        virtual SceneEntity createEmptyEntity() = 0;
        virtual SceneEntity duplicateEmptyEntity(SceneEntity entity) = 0;
        virtual void setBefore(SceneEntity sourceEntity, SceneEntity target) = 0;
        virtual void removeEntityChild(SceneEntity entity) = 0;
        virtual bool isRunning() const = 0;
        virtual SceneEntity getByUUID(UUID uuid) = 0;
        virtual void registerOnDeleteFinish(std::function<void()>&& callback) = 0;

        virtual void uiSelectedEntities(std::set<ITreeItem::Ptr>& uiItems, bool isAll) = 0;

        virtual void exportProject(const ExportOptions& exportOptions) = 0;
        virtual const std::vector<class_id>& getCommandStack() const = 0;
    };

} // namespace editor