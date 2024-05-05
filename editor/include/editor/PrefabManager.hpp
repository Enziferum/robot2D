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
        PrefabManager(const PrefabManager& other) = delete;
        PrefabManager& operator=(const PrefabManager& other) = delete;
        PrefabManager(PrefabManager&& other) = delete;
        PrefabManager& operator=(PrefabManager&& other) = delete;
        ~PrefabManager() = default;

        bool addPrefab(SceneEntity entity, const std::string& directoryPath);
        Prefab::Ptr loadPrefab(UIInteractor::Ptr interactor, const std::string& path);
        bool hasPrefab(UUID prefabID) const;
        bool hasPrefab(std::string localPath) const;

        bool savePrefab(Prefab::Ptr prefab);
        bool savePrefab(UUID uuid);

        Prefab::Ptr findPrefab(UUID prefabID);

        bool deletePrefab(UUID prefabID);
    private:
        Prefab::Ptr findPrefab(const std::string& path);
    private:
        std::unordered_map<UUID, Prefab::Ptr> m_prefabs;
    };
} // namespace editor