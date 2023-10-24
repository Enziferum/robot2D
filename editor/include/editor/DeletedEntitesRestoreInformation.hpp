#pragma once
#include <vector>
#include "Uuid.hpp"

namespace editor {

    struct DeletedEntitiesRestoreInformation {
        struct RestoreInfo {
            bool first;
            bool child;
            UUID uuid;
        };


        void push(UUID uuid, bool first, bool child) {
            RestoreInfo info{first, child, uuid};
            anchorEntitiesUuids.emplace_back(info);
        }

        std::vector<RestoreInfo> anchorEntitiesUuids;
    };

    struct DeletedEntitiesRestoreUIInformation {

    };

}