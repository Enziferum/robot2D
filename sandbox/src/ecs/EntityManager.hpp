/*********************************************************************
(c) Alex Raag 2021
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
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>

#include "Bitmask.hpp"
#include "Component.hpp"
#include "Defines.hpp"
#include "Entity.hpp"

namespace ecs {
    using ComponentContainer = std::vector<Component::Ptr>;
    using EntityData = std::pair<Bitmask, ComponentContainer>;
    using EntityContainer = std::unordered_map<EntityID, EntityData>;

    class SystemManager;
    class EntityManager {
    public:
        EntityManager();
        ~EntityManager() = default;

        void setSystemManager(SystemManager* systemManager);

        // allows register component outside manager
        template<typename T>
        bool registerComponent(const ComponentID& id);

        Entity addEntity(const Bitmask& bitmask);

        bool addComponent(const EntityID& entityId, const ComponentID& id);

        template<typename T>
        T* getComponent(const EntityID& entityId, const ComponentID& id);

//        template<typename T>
//        const T* getComponent(const EntityID& entityId, const ComponentID& id) const;

        bool hasComponent(const EntityID& entityId, const ComponentID& id);

        bool hasComponent(const EntityID& entityId, const ComponentID& id) const;
    private:
        EntityID m_entityCounter;
        EntityContainer m_entityContainer;
        std::unordered_map<ComponentID, std::function<Component::Ptr()>> m_componentFactory;
        SystemManager* m_systemManager;
    };


    template<typename T>
    T* Entity::getComponent(const ComponentID& id) {
        return m_entityManager -> getComponent<T>(m_id, id);
    }

    template<typename T>
    const T* Entity::getComponent(const ComponentID &id) const {
        return m_entityManager -> getComponent<T>(m_id, id);
    }

    template<typename T>
    bool EntityManager::registerComponent(const ComponentID& id) {
        if(!std::is_base_of<Component, T>::value) {
            return false;
        }

        m_componentFactory[id] = []() {
            return std::make_shared<T>();
        };
        return true;
    }

    template<typename T>
    T* EntityManager::getComponent(const EntityID& entityId, const ComponentID& componentId) {
        if(!hasComponent(entityId, componentId))
            return nullptr;

        auto& components = m_entityContainer[entityId].second;
        auto it = std::find_if(components.begin(), components.end(), [&componentId](const Component::Ptr& ptr) {
            return ptr->getType() == static_cast<uint32_t>(componentId);
        });

        if(it == components.end())
            return nullptr;

        return dynamic_cast<T*>((*it).get());
    }

//    template<typename T>
//    const T* EntityManager::getComponent(const EntityID& entityId, const ComponentID& componentId) const {
//        if(!hasComponent(entityId, componentId))
//            return nullptr;
//
//        const auto& components = m_entityContainer[entityId].second;
//        auto it = std::find_if(components.begin(), components.end(), [&componentId](const Component::Ptr& ptr) {
//            return ptr->getType() == static_cast<uint32_t>(componentId);
//        });
//
//        if(it == components.end())
//            return nullptr;
//
//        return dynamic_cast<T*>((*it).get());
//    }


}