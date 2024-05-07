/*********************************************************************
(c) Alex Raag 2023
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
#include <cassert>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <memory>

#include <robot2D/Config.hpp>

#include "Bitmask.hpp"
#include "Component.hpp"
#include "Defines.hpp"
#include "Entity.hpp"
#include "ComponentContainer.hpp"

namespace robot2D::ecs {

    class SystemManager;
    class Scene;
    class EntityManager {
    public:
        EntityManager(ComponentManager& componentManager, Scene* scene);
        EntityManager() = delete;
        EntityManager(const EntityManager& other) = delete;
        EntityManager& operator=(const EntityManager& other) = delete;
        EntityManager(EntityManager&& other) = delete;
        EntityManager& operator=(EntityManager&& other) = delete;
        ~EntityManager() = default;

        Entity createEntity(bool needAddToScene = true);

        template<typename T>
        void addComponent(Entity entity, T component);

        template<typename T>
        void removeComponent(Entity entity);

        template<typename T, typename ... Args>
        T& addComponent(Entity, Args&&...args);

        template<typename T>
        T& getComponent(Entity entity);

        template<typename T>
        const T& getComponent(Entity entity) const ;

        template<typename T>
        bool hasComponent(Entity entity);

        template<typename T>
        bool hasComponent(Entity entity) const;

        bool removeEntity(Entity entity);

        bool restoreEntity(Entity entity);

        void removeEntityFromScene(Entity entity);

        bool entityDestroyed(Entity entity);

        Bitmask getComponentBitmask(Entity entity);

        Entity duplicateEntity(robot2D::ecs::Entity entity);
    private:
        void markDestroyed(Entity entity);

        /// \brief allows to get Container of special type
        template<typename T>
        ComponentContainer<T>& getContainer();

        void addEntityToScene(robot2D::ecs::Entity);

        /// \brief deep copy. IMPORTANT: copies only valid ( not destroyed entities and components ).
        /// also it means components must be copyable!!!
        bool cloneSelf(EntityManager& cloneManager);

        bool clearSelf();
    private:
        friend class Scene;

        EntityID m_entityCounter;
        ComponentManager& m_componentManager;

        std::vector<IContainer::Ptr> m_componentContainers;
        std::vector<IContainer::Ptr> m_componentContainersDeleteBuffer;

        std::unordered_map<EntityID, bool> m_destroyFlags;
        std::unordered_map<EntityID, Bitmask> m_componentMasks;

        Scene* m_ownerScene{ nullptr };
    };

    template<typename T>
    ComponentContainer<T>& EntityManager::getContainer() {
        const auto componentID = m_componentManager.getID<T>();
        if(m_componentContainers[componentID] == nullptr) {
            m_componentContainers[componentID] = std::make_shared<ComponentContainer<T>>(componentID);
        }
        return *(dynamic_cast<ComponentContainer<T>*>(m_componentContainers[componentID].get()));
    }


    template<typename T, typename... Args>
    T& Entity::addComponent(Args&& ... args) {
       return m_entityManager -> addComponent<T>(*this, std::forward<Args>(args)...);
    }

    template<typename T>
    bool Entity::hasComponent() const {
        if(!m_entityManager)
            return false;
        return m_entityManager -> hasComponent<T>(*this);
    }

    template<typename T>
    T& Entity::getComponent() {
        return m_entityManager -> getComponent<T>(*this);
    }

    template<typename T>
    const T& Entity::getComponent() const {
        return m_entityManager -> getComponent<T>(*this);
    }

    template<typename T>
    void Entity::removeComponent() {
        m_entityManager -> removeComponent<T>(*this);
    }

    template<typename T>
    void EntityManager::addComponent(Entity entity, T component) {
        const auto& componentID = m_componentManager.getID<T>();
        const auto& entityID = entity.getIndex();

        ComponentContainer<T>& container = getContainer<T>();
        container[entityID] = component;
        m_componentMasks[entityID].turnOnBit(componentID);
        addEntityToScene(entity);
    }

    template<typename T, typename... Args>
    T& EntityManager::addComponent(Entity entity, Args &&... args) {
        T component{std::forward<Args>(args)...};
        addComponent(entity, component);
        return getComponent<T>(entity);
    }

    template<typename T>
    T& EntityManager::getComponent(Entity entity) {
        const auto& componentID = m_componentManager.getID<T>();
        const auto& index = entity.getIndex();

        auto* container = dynamic_cast<ComponentContainer<T>*>(m_componentContainers[componentID].get());
        // TODO: get Valid String of T
        assert(container != nullptr && "Don't have container of Type");
        return container -> at(index);
    }

    template<typename T>
    const T& EntityManager::getComponent(Entity entity) const {
        const auto& componentID = m_componentManager.getID<T>();
        const auto& index = entity.getIndex();

        auto* container = dynamic_cast<ComponentContainer<T>*>(m_componentContainers[componentID].get());
        // TODO: get Valid String of T
        assert(container != nullptr && "Don't have container of Type");
        return container -> at(index);
    }


    template<typename T>
    bool EntityManager::hasComponent(Entity entity) {
        const auto& componentID = m_componentManager.getID<T>();
        const auto& index = entity.getIndex();

        return m_componentMasks[index].getBit(componentID);
    }


    template<typename T>
    bool EntityManager::hasComponent(Entity entity) const {
        const auto& componentID = m_componentManager.getID<T>();
        const auto& index = entity.getIndex();
        return m_componentMasks.at(index).getBit(componentID);
    }

    template<typename T>
    void EntityManager::removeComponent(Entity entity) {
        if(!hasComponent<T>(entity))
            return;

        const auto entityID = entity.getIndex();
        ComponentContainer<T>& container = getContainer<T>();

        const auto componentID = m_componentManager.getID<T>();
        container.remove(entityID);
        m_componentMasks[entityID].clear(componentID);
    }

}