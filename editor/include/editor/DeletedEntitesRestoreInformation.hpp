#pragma once
#include <vector>
#include <optional>
#include <editor/SceneEntity.hpp>

namespace editor {

    struct DeletedEntitiesRestoreInformation {
        struct RestoreInfo {
            bool first { false };
            bool isChained{ false };
            bool child{ false };
            SceneEntity anchorEntity;
            SceneEntity entity;
            std::optional<UUID> nextSiblingID;
        };

        /// TODO(a.raag): correct valid statement
        bool valid() const { return true; }
        bool hasItems() const { return !m_anchorEntitiesUuids.empty(); }

        RestoreInfo& getLast() { return m_anchorEntitiesUuids.back(); }

        std::vector<RestoreInfo>& getInfos() { return m_anchorEntitiesUuids; }

        void push(SceneEntity anchorEntity,
                  SceneEntity entity, bool first, bool isChained, bool child) {
            RestoreInfo info{first, isChained, child, anchorEntity, entity};
            m_anchorEntitiesUuids.emplace_back(info);
        }

        void push(RestoreInfo&& restoreInfo) { m_anchorEntitiesUuids.emplace_back(std::move(restoreInfo)); }
    private:
        std::vector<RestoreInfo> m_anchorEntitiesUuids;
    };

}
