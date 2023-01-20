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
#include <cassert>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <memory>

#include "Bitmask.hpp"
#include "Component.hpp"
#include "Defines.hpp"
#include "Entity.hpp"

namespace robot2D::ecs {

    // public non template API
    class IContainer {
    public:
        using Ptr = std::shared_ptr<IContainer>;
    public:
        virtual ~IContainer() = default;
        virtual size_t getSize() const = 0;
        virtual void resize(const size_t& newSize) = 0;

        virtual bool removeEntity(EntityID&& entityId) = 0;
    };

    // Container for component of special type
    template<typename T>
    class ComponentContainer: public IContainer {
    public:
        using Ptr = std::shared_ptr<ComponentContainer<T>>;
    public:
        ComponentContainer(): m_components(256) {}
        ~ComponentContainer() = default;

        size_t getSize() const override {
            return m_components.size();
        }

        void resize(const size_t& newSize) override {
           // m_components.resize(newSize);
        }

        T& operator[](std::size_t index) {
         //   assert(index < m_components.size() && "Component Container index > size()");
            return m_components[index];
        }

        const T& operator[](std::size_t index) const {
           // assert(index < m_components.size() && "Component Container index > size()");
            return m_components[index];
        }

        T& at(std::size_t index) {
            return m_components.at(index);
        }

        const T& at(std::size_t index) const {
           return m_components.at(index);
        }

        void remove(std::size_t index) {
            if(index >= m_components.size())
                return;
         //   m_components.erase(m_components.begin() + index);
        }

        bool removeEntity(robot2D::ecs::EntityID&& entityId) override {
            return static_cast<bool>(m_components.erase(entityId));
        }
    private:
        std::unordered_map<EntityID, T> m_components;
    };

    class SystemManager;
    class EntityManager {
    public:
        EntityManager(ComponentManager& componentManager);
        ~EntityManager() = default;

        Entity createEntity();

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

        bool entityDestroyed(Entity entity);

        Bitmask getComponentBitmask(Entity entity);

    private:
        void markDestroyed(Entity entity);
    private:
        EntityID m_entityCounter;
        friend class Scene;
        ComponentManager& m_componentManager;
        std::vector<IContainer::Ptr> m_componentContainers;
        std::unordered_map<EntityID, Bitmask> m_componentMasks;

        std::vector<bool> m_destroyFlags;

        // allows to get Container of special type
        template<typename T>
        ComponentContainer<T>& getContainer();
    };

    template<typename T>
    ComponentContainer<T>& EntityManager::getContainer() {
        const auto componentID = m_componentManager.getID<T>();
        if(m_componentContainers[componentID] == nullptr) {
            m_componentContainers[componentID] = std::make_shared<ComponentContainer<T>>();
        }
        return *(dynamic_cast<ComponentContainer<T>*>(m_componentContainers[componentID].get()));
    }


    template<typename T, typename... Args>
    T& Entity::addComponent(Args &&... args) {
       return m_entityManager -> addComponent<T>(*this, std::forward<Args>(args)...);
    }

    template<typename T>
    bool Entity::hasComponent() {
        if(!m_entityManager)
            return false;
        return m_entityManager -> hasComponent<T>(*this);
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
        // need for bitmask
        const auto& componentID = m_componentManager.getID<T>();
        // identifier for storage
        const auto& entityID = entity.getIndex();
        // 1. get ComponentContainer by identifier
        // 2. add component
        // 3. update bitmask

        ComponentContainer<T>& container = getContainer<T>();
        container[entityID] = component;
        m_componentMasks[entityID].turnOnBit(componentID);
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
        // 1. get ComponentContainer by identifier
        // 2. add component
        // 3. update bitmask

        ComponentContainer<T> container = getContainer<T>();

        const auto componentID = m_componentManager.getID<T>();
        container.remove(entityID);
        m_componentMasks[entityID].clear(componentID);
    }

}