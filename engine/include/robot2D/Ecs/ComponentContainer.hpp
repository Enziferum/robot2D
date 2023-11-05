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

#include <unordered_map>
#include <memory>
#include <cassert>
#include <type_traits>

#include "Defines.hpp"
#include "Component.hpp"

namespace robot2D::ecs {

    /// \brief Public non template API
    class IContainer {
    public:
        using Ptr = std::shared_ptr<IContainer>;
    public:
        IContainer(ComponentManager::ID id): m_containerID{id} {}
        virtual ~IContainer() = default;
        virtual size_t getSize() const = 0;

        virtual bool hasEntity(EntityID entityId) const = 0;
        virtual bool duplicate(EntityID from, EntityID to) = 0;
        virtual bool removeEntity(EntityID&& entityId) = 0;


        virtual IContainer::Ptr cloneEmpty() = 0;
        /// \brief Clone Component From One Container to Other
        virtual bool cloneTo(IContainer::Ptr target, EntityID fromEntity) = 0;

        ComponentManager::ID getID() const { return m_containerID; }
    protected:
        /// brief indificate type of Component stored in Container
        ComponentManager::ID m_containerID;
    };

    class CustomDestroyComponent {
    public:
        virtual ~CustomDestroyComponent() = 0;
        virtual void destroy() = 0;
    };


    /// \brief Container for component of special type
    template<typename T>
    class ROBOT2D_EXPORT_API ComponentContainer: public IContainer {
    public:
        using Ptr = std::shared_ptr<ComponentContainer<T>>;
    public:
        ComponentContainer(ComponentManager::ID id): IContainer(id), m_components(256) {}
        ComponentContainer(const ComponentContainer& other);
        ComponentContainer& operator=(const ComponentContainer& other);
        ComponentContainer(ComponentContainer&& other);
        ComponentContainer& operator=(ComponentContainer&& other);
        ~ComponentContainer() override = default;

        size_t getSize() const override {
            return m_components.size();
        }

        T& operator[](std::size_t index) {
            // assert(index < m_components.size() && "Component Container index > size()");
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

        bool hasEntity(robot2D::ecs::EntityID entityId) const override {
            return m_components.find(entityId) != m_components.end();
        }

        bool duplicate(robot2D::ecs::EntityID from, robot2D::ecs::EntityID to) override {
            m_components[to] = m_components[from];
            return true;
        }

        void remove(std::size_t index) {
            if(index >= m_components.size())
                return;
           // m_components.erase(m_components.begin() + index);
        }


        bool removeEntity(robot2D::ecs::EntityID&& entityId) override {
            // TODO(a.raag) make possible to component make custom destroy without dynamic_cast,
            //  maybe create removeEntityWithCallback(destroyCallback);
//            if(auto destroyComponent = dynamic_cast<CustomDestroyComponent*>(&m_components[entityId]))
//                destroyComponent -> destroy();
            return static_cast<bool>(m_components.erase(entityId));
        }

        IContainer::Ptr cloneEmpty() override {
            return std::make_shared<ComponentContainer<T>>(m_containerID);
        }

        bool cloneTo(IContainer::Ptr target, EntityID fromEntity) override {
            if(m_containerID != target -> getID())
                return false;

            if(ComponentContainer<T>::Ptr targetRealContainer =
                    std::dynamic_pointer_cast<ComponentContainer<T>>(target)) {
                targetRealContainer -> cloneComponent(fromEntity, m_components[fromEntity]);
                return true;
            }
            else
                return false;
        }
    private:
        void cloneComponent(EntityID fromEntity, T component) {
            m_components[fromEntity] = component;
        }
    private:
        std::unordered_map<EntityID, T> m_components;
    };


}