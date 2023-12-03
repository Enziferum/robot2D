#pragma once
#include <vector>
#include <robot2D/Ecs/Entity.hpp>
#include <editor/panels/TreeHierarchy.hpp>

namespace editor {

    struct DeletedEntitiesRestoreInformation {
        struct RestoreInfo {
            bool first;
            bool child;
            robot2D::ecs::Entity anchorEntity;
            robot2D::ecs::Entity entity;
        };

        /// TODO(a.raag): correct valid statement
        bool valid() const { return true; }

        void push(robot2D::ecs::Entity anchorEntity,
                  robot2D::ecs::Entity entity, bool first, bool child) {
            RestoreInfo info{first, child, anchorEntity, entity};
            anchorEntitiesUuids.emplace_back(info);
        }

        std::vector<RestoreInfo> anchorEntitiesUuids;
    };

    class DeletedEntitiesRestoreUIInformation {
    public:
        struct RestoreInfo {
            bool first;
            bool isChained;
            ITreeItem::Ptr target;
            ITreeItem::Ptr anchor;
        };
        /// TODO(a.raag): correct valid statement
        bool valid() const { return true; }

        bool hasItems() const { return !anchorItems.empty(); }

        RestoreInfo& getLast() { return anchorItems.back(); }

        void push(ITreeItem::Ptr target, ITreeItem::Ptr anchor, bool first = false, bool isChained = false) {
            auto info = RestoreInfo{first, isChained, target, anchor};
            anchorItems.push_back(info);
        }
    //private:


        std::vector<RestoreInfo> anchorItems;
    };

}