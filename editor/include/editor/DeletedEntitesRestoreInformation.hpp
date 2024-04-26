#pragma once
#include <vector>
#include <editor/SceneEntity.hpp>
#include <editor/panels/TreeHierarchy.hpp>

namespace editor {

    struct DeletedEntitiesRestoreInformation {
        struct RestoreInfo {
            bool first;
            bool isChained;
            bool child;
            SceneEntity anchorEntity;
            SceneEntity entity;
        };

        /// TODO(a.raag): correct valid statement
        bool valid() const { return true; }
        bool hasItems() const { return !anchorEntitiesUuids.empty(); }

        RestoreInfo& getLast() { return anchorEntitiesUuids.back(); }

        std::vector<RestoreInfo>& getInfos() { return anchorEntitiesUuids; }

        void push(SceneEntity anchorEntity,
                  SceneEntity entity, bool first, bool isChained, bool child) {
            RestoreInfo info{first, isChained, child, anchorEntity, entity};
            anchorEntitiesUuids.emplace_back(info);
        }
    private:
        std::vector<RestoreInfo> anchorEntitiesUuids;
    };

    class DeletedEntitiesRestoreUIInformation {
    public:
        struct RestoreInfo {
            bool first { false };
            bool isChained { false };
            ITreeItem::Ptr target { nullptr };
            ITreeItem::Ptr anchor { nullptr };
        };

        /// TODO(a.raag): correct valid statement
        bool valid() const noexcept { return true; }
        bool hasItems() const noexcept { return !m_anchorItems.empty(); }

        const RestoreInfo& getLast() const { return m_anchorItems.back(); }

        void push(RestoreInfo&& restoreInfo) {
            m_anchorItems.emplace_back(std::move(restoreInfo));
        }

        const std::vector<RestoreInfo>& getItems() const { return m_anchorItems; }
    private:
        std::vector<RestoreInfo> m_anchorItems;
    };

}