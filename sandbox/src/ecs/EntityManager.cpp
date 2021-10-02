/*********************************************************************
(c) Alex Raag 2021
https://github.com/Enziferum
ZombieArena - Zlib license.
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

#include "EntityManager.hpp"
#include "SystemManager.hpp"

namespace ecs {

    EntityManager::EntityManager(): m_entityCounter(0),
                                    m_entityContainer(),
                                    m_componentFactory() {

    }

    bool EntityManager::hasComponent(const EntityID &entityId, const ComponentID& componentId) {
        if(m_entityContainer.find(entityId) == m_entityContainer.end())
            return false;
        return m_entityContainer.at(entityId).first.getBit(componentId);
    }


    bool EntityManager::addComponent(const EntityID& entityId, const ComponentID& componentId) {
        // have already component or no entity in container
        if(hasComponent(entityId, componentId))
            return false;

        // no component need register before todo throw error with message
        if(m_componentFactory.find(componentId) == m_componentFactory.end())
            return false;


        Component::Ptr component = m_componentFactory.at(componentId)();
        m_entityContainer.at(entityId).second.emplace_back(component);
        m_entityContainer.at(entityId).first.turnOnBit(componentId);
        m_systemManager -> entityModified({this, entityId}, m_entityContainer.at(entityId).first);
        return true;
    }


    Entity EntityManager::addEntity(const Bitmask& bitmask) {
        EntityID entity = m_entityCounter;
        if(!m_entityContainer.emplace(entity, EntityData(0, ComponentContainer())).second)
            return Entity{};
        ++m_entityCounter;

        for(int it = 0; it < maxComponents; ++it) {
            if(bitmask.getBit(it))
                addComponent(entity, it);
        }
        m_systemManager -> entityModified({this, entity}, bitmask);
        return Entity{this, entity};
    }

    void EntityManager::setSystemManager(SystemManager* systemManager) {
        m_systemManager = systemManager;
    }

    bool EntityManager::hasComponent(const EntityID &entityId, const ComponentID& componentId) const {
        if(m_entityContainer.find(entityId) == m_entityContainer.end())
            return false;
        return m_entityContainer.at(entityId).first.getBit(componentId);
    }


}