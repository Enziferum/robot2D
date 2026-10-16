/*********************************************************************
   (c) Alex Raag 2026
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

#include <stdexcept>
#include <robot2D/Util/Logger.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/PrefabManager.hpp>
#include <editor/serializers/PrefabSerializer.hpp>
#include <editor/FileApi.hpp>
#include <editor/Components.hpp>
#include <editor/async/SceneLoadTask.hpp>



namespace editor {

    namespace {
        const char* prefabExtension = ".prefab";
    }

    bool PrefabManager::addPrefab(UIInteractor::Ptr interactor, SceneEntity entity, const std::string& directoryPath) {
        if(!interactor || !entity || !entity.hasComponent<TagComponent>())
            return false;
        const auto filename = std::filesystem::u8path(entity.getName() + prefabExtension);
        if(filename.has_parent_path())
            return false;
        const auto fullPath = (std::filesystem::u8path(directoryPath) / filename).u8string();
        std::error_code error;
        if(std::filesystem::exists(std::filesystem::u8path(fullPath), error)) {
            RB_EDITOR_ERROR("Prefab already exists; use Save to update the asset: {0}", fullPath);
            return false;
        }
        if(error)
            return false;
        // A cached prefab owns its template; it must never retain the selected scene entity.
        auto prefab = std::make_shared<Prefab>(fullPath, entity, UUID());
        prefab -> setProjectPath(interactor -> getAssociatedProjectPath());
        PrefabSerializer serializer;
        auto scripts = interactor -> getScriptInteractor();
        if(!serializer.serialize(prefab, scripts) || !serializer.deserialize(prefab, scripts)) {
            RB_EDITOR_ERROR("Can't create prefab: {0}", serializer.getErrorMessage());
            return false;
        }
        loadSceneAssets(prefab -> getTemplateScene());
        if(!entity.hasComponent<PrefabComponent>())
            entity.addComponent<PrefabComponent>();
        entity.getComponent<PrefabComponent>().prefabUUID = prefab -> getUUID();
        entity.markModified();
        m_prefabs[prefab -> getUUID()] = std::move(prefab);
        return true;
    }

    Prefab::Ptr PrefabManager::loadPrefab(UIInteractor::Ptr interactor, const std::string& path) {
        if(!interactor)
            return nullptr;
        return loadPrefab(interactor -> getAssociatedProjectPath(), path, interactor -> getScriptInteractor());
    }

    Prefab::Ptr PrefabManager::loadPrefab(const std::string& projectPath, const std::string& path,
        IScriptInteractorFrom::Ptr scripts) {
        std::string resolved;
        try {
            resolved = resolveProjectAsset(projectPath, path);
            if(std::filesystem::u8path(resolved).extension() != ".prefab")
                throw std::invalid_argument("Expected .prefab asset");
        }
        catch(const std::exception& error) {
            RB_EDITOR_ERROR("Can't load prefab: path='{}'; reason='{}'", path, error.what());
            return nullptr;
        }
        if(auto cached = findPrefab(resolved))
            return cached;
        auto prefab = std::make_shared<Prefab>();
        prefab -> setPath(resolved);
        prefab -> setProjectPath(projectPath);
        PrefabSerializer serializer;
        if(!serializer.deserialize(prefab, scripts)) {
            RB_EDITOR_ERROR("Can't load prefab: path='{}'; reason='{}'", resolved, serializer.getErrorMessage());
            return nullptr;
        }
        const auto found = m_prefabs.find(prefab -> getUUID());
        if(found != m_prefabs.end()) {
            RB_EDITOR_ERROR("Another prefab with the same asset UUID is already cached: {0}", path);
            return nullptr;
        }
        loadSceneAssets(prefab -> getTemplateScene());
        m_prefabs[prefab -> getUUID()] = prefab;
        return prefab;
    }

    Prefab::Ptr PrefabManager::findPrefab(UUID prefabID) {
        const auto found = m_prefabs.find(prefabID);
        return found == m_prefabs.end() ? nullptr : found -> second;
    }

    Prefab::Ptr PrefabManager::findPrefab(const std::string& path) const {
        for(const auto& prefab: m_prefabs) {
            std::error_code error;
            if(std::filesystem::equivalent(std::filesystem::u8path(prefab.second -> getPath()),
                std::filesystem::u8path(path), error) && !error)
                return prefab.second;
        }

        return nullptr;
    }

    bool PrefabManager::hasPrefab(UUID prefabID) const {
        return m_prefabs.find(prefabID) != m_prefabs.end();
    }

    bool PrefabManager::hasPrefab(std::string path) const {
        return static_cast<bool>(findPrefab(path));
    }

    bool PrefabManager::hasChanges() const {
        for(const auto& [id, prefab]: m_prefabs)
            if(prefab -> getTemplateScene() && prefab -> getTemplateScene() -> hasChanges())
                return true;
        return false;
    }

    bool PrefabManager::saveChanges(UIInteractor::Ptr interactor) {
        for(const auto& [id, prefab]: m_prefabs) {
            if(prefab -> getTemplateScene() && prefab -> getTemplateScene() -> hasChanges()) {
                if(!savePrefab(interactor, prefab)) {
                    RB_EDITOR_ERROR("Can't save modified prefab: {0}", prefab -> getPath());
                    return false;
                }
                interactor -> applyPrefabChanges(id, prefab -> getEntity());
            }
        }
        return true;
    }

    void PrefabManager::discardChanges() {
        for(auto it = m_prefabs.begin(); it != m_prefabs.end();) {
            if(it -> second -> getTemplateScene() && it -> second -> getTemplateScene() -> hasChanges())
                it = m_prefabs.erase(it);
            else
                ++it;
        }
    }

    bool PrefabManager::deletePrefab(UUID prefabID) {
        return true;
    }

    bool PrefabManager::savePrefab(UIInteractor::Ptr interactor, Prefab::Ptr prefab) {
        if(!prefab || !interactor)
            return false;
        auto scriptInteractor = interactor -> getScriptInteractor();
        PrefabSerializer serializer;
        return serializer.serialize(prefab, scriptInteractor);
    }

    bool PrefabManager::savePrefab(UIInteractor::Ptr interactor, UUID uuid) {
        if(!interactor)
            return false;
        auto prefab = findPrefab(uuid);
        if(!prefab)
            return false;

        PrefabSerializer serializer;
        auto scriptInteractor = interactor -> getScriptInteractor();
        return serializer.serialize(prefab, scriptInteractor);
    }


} // namespace editor
