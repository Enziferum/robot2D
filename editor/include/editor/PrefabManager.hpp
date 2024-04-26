#pragma once
#include <string>
#include <unordered_map>
#include <editor/SceneEntity.hpp>

#include "Prefab.hpp"
#include "Uuid.hpp"
#include "Scene.hpp"
#include "UIInteractor.hpp"

namespace editor {
    class PrefabManager {
    public:
        PrefabManager() = default;
        ~PrefabManager() = default;

        bool addPrefab(SceneEntity entity, const std::string& directoryPath);
        Prefab::Ptr loadPrefab(UIInteractor::Ptr interactor, const std::string& path);
        bool hasPrefab(UUID prefabID) const;
        bool hasPrefab(std::string localPath) const;

        bool savePrefab(Prefab::Ptr prefab);
        bool savePrefab(UUID uuid);

        Prefab::Ptr findPrefab(UUID prefabID);
        Prefab::Ptr findPrefab(std::string localPath);
        bool deletePrefab(UUID prefabID);
    private:
        std::unordered_map<UUID, Prefab::Ptr> m_prefabs;
    };
}