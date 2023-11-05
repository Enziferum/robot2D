#pragma once
#include <string>
#include <unordered_map>
#include <robot2D/Ecs/Entity.hpp>

#include "Prefab.hpp"
#include "Uuid.hpp"
#include "Scene.hpp"
#include "UIInteractor.hpp"

namespace editor {
    class PrefabManager {
    public:
        PrefabManager() = default;
        ~PrefabManager() = default;

        bool addPrefab(robot2D::ecs::Entity entity, const std::string& directoryPath);
        Prefab::Ptr loadPrefab(UIInteractor::Ptr interactor, const std::string& path);
        bool hasPrefab(UUID prefabID) const;
        bool hasPrefab(std::string localPath) const;

        bool savePrefab(Prefab::Ptr prefab);

        Prefab::Ptr findPrefab(UUID prefabID);
        Prefab::Ptr findPrefab(std::string localPath);
        bool deletePrefab(UUID prefabID);
    private:
        std::unordered_map<UUID, Prefab::Ptr> m_prefabs;
    };
}