/*********************************************************************
(c) Alex Raag 2024
https://github.com/Enziferum
robot2D - Zlib license.
This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <robot2D/Util/Logger.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/PrefabManager.hpp>
#include <editor/serializers/PrefabSerializer.hpp>
#include <editor/FileApi.hpp>
#include <editor/Components.hpp>



namespace editor {

    namespace {
        const char* prefabExtension = ".prefab";
    }

    bool PrefabManager::addPrefab(UIInteractor::Ptr interactor, SceneEntity entity, const std::string& directoryPath) {
        const auto& uuid = entity.getComponent<IDComponent>().ID;
        const auto& name = entity.getComponent<TagComponent>().getTag();
        std::string filePath = name + prefabExtension;
        std::string fullPath = combinePath(directoryPath, filePath);

        m_prefabs[uuid] = std::make_shared<Prefab>(fullPath, entity, UUID());

        auto scriptInteractor = interactor -> getScriptInteractor();
        if(!scriptInteractor)
            return false;
        PrefabSerializer serializer;
        return serializer.serialize(m_prefabs[uuid], scriptInteractor);
    }

    Prefab::Ptr PrefabManager::loadPrefab(UIInteractor::Ptr interactor, const std::string& path) {

        Prefab::Ptr prefab{ nullptr };
        prefab = findPrefab(path);
        if(prefab)
            return prefab;

        prefab = std::make_shared<Prefab>();
        prefab -> setEntity(interactor -> createEmptyEntity());
        prefab -> setPath(path);

        PrefabSerializer serializer;
        auto scriptInteractor = interactor -> getScriptInteractor();
        bool ok = serializer.deserialize(prefab, scriptInteractor);
        if(!ok) {
            RB_EDITOR_ERROR("PrefabManager: can't load prefab");
            return nullptr;
        }

        prefab -> addUUID();

        auto uuid = prefab -> getUUID();
        m_prefabs[uuid] = prefab;

        return m_prefabs[uuid];
    }

    Prefab::Ptr PrefabManager::findPrefab(UUID prefabID) {
        return nullptr;
    }

    Prefab::Ptr PrefabManager::findPrefab(const std::string& path) {
        for(auto& prefab: m_prefabs) {
           if(prefab.second -> getPath() == path)
             return prefab.second;
        }

        return nullptr;
    }

    bool PrefabManager::deletePrefab(UUID prefabID) {
        return true;
    }

    bool PrefabManager::savePrefab(UIInteractor::Ptr interactor, Prefab::Ptr prefab) {
        if(!prefab)
            return false;
        const auto& uuid = prefab -> getUUID();

        auto scriptInteractor = interactor -> getScriptInteractor();
        if(!scriptInteractor)
            return false;
        PrefabSerializer serializer;
        return serializer.serialize(m_prefabs[uuid], scriptInteractor);
    }

    bool PrefabManager::savePrefab(UIInteractor::Ptr interactor, UUID uuid) {
        if(m_prefabs.find(uuid) == m_prefabs.end())
            return false;

        PrefabSerializer serializer;
        auto scriptInteractor = interactor -> getScriptInteractor();
        if(!scriptInteractor)
            return false;
        return serializer.serialize(m_prefabs[uuid], scriptInteractor);
    }


} // namespace editor