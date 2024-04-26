#include <robot2D/Util/Logger.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/PrefabManager.hpp>
#include <editor/serializers/PrefabSerializer.hpp>
#include <editor/FileApi.hpp>
#include <editor/Components.hpp>



namespace editor {

    namespace {
        constexpr char* prefabExtension = ".prefab";
    }

    bool PrefabManager::addPrefab(SceneEntity entity, const std::string& directoryPath) {
        const auto& uuid = entity.getComponent<IDComponent>().ID;
        const auto& name = entity.getComponent<TagComponent>().getTag();
        std::string filePath = name + prefabExtension;
        std::string fullPath = combinePath(directoryPath, filePath);

        m_prefabs[uuid] = std::make_shared<Prefab>();
        m_prefabs[uuid] -> entity = entity;
        m_prefabs[uuid] -> localPath = fullPath;
        m_prefabs[uuid] -> prefabUUID = UUID();

        PrefabSerializer serializer;
        return  serializer.serialize(m_prefabs[uuid], fullPath);;
    }

    Prefab::Ptr PrefabManager::loadPrefab(UIInteractor::Ptr interactor, const std::string& path) {

        Prefab::Ptr prefab{ nullptr };
        prefab = findPrefab(path);
        if(prefab)
            return prefab;

        prefab = std::make_shared<Prefab>();
        prefab -> entity = interactor -> createEmptyEntity();
        prefab -> localPath = path;

        PrefabSerializer serializer;
        bool ok = serializer.deserialize(prefab, path);
        if(!ok) {
            RB_EDITOR_ERROR("Can't load Prefab File");
            return nullptr;
        }

        prefab -> entity.addComponent<PrefabComponent>().prefabUUID = prefab -> prefabUUID;

        auto uuid = prefab -> prefabUUID;
        m_prefabs[uuid] = prefab;


        return m_prefabs[uuid];
    }

    Prefab::Ptr PrefabManager::findPrefab(UUID prefabID) {
        return nullptr;
    }

    Prefab::Ptr PrefabManager::findPrefab(std::string localPath) {
        for(auto& prefab: m_prefabs) {
            if(prefab.second -> localPath == localPath)
                return prefab.second;
        }

        return nullptr;
    }

    bool PrefabManager::deletePrefab(UUID prefabID) {
        return true;
    }

    bool PrefabManager::savePrefab(Prefab::Ptr prefab) {
        if(!prefab)
            return false;
        const auto& uuid = prefab -> entity.getComponent<IDComponent>().ID;

        PrefabSerializer serializer;
        return serializer.serialize(m_prefabs[uuid], prefab -> localPath);;
    }

    bool PrefabManager::savePrefab(UUID uuid) {
        if(m_prefabs.find(uuid) == m_prefabs.end())
            return false;

        PrefabSerializer serializer;
        return serializer.serialize(m_prefabs[uuid], m_prefabs[uuid] -> localPath);
    }


} // namespace editor