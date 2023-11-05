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

    struct DeletedEntitiesRestoreUIInformation {
        struct RestoreInfo {
            bool first;
            ITreeItem::Ptr target;
            ITreeItem::Ptr anchor;
        };

        /// TODO(a.raag): correct valid statement
        bool valid() const { return true; }

        void push(ITreeItem::Ptr target, ITreeItem::Ptr anchor, bool first) {
            auto info = RestoreInfo{first, target, anchor};
            anchorItems.push_back(info);
        }

        std::vector<RestoreInfo> anchorItems;
    };

}