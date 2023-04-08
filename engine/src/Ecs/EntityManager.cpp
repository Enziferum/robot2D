/*********************************************************************
(c) Alex Raag 2023
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

#include <algorithm>
#include <robot2D/Ecs/EntityManager.hpp>
#include <robot2D/Ecs/SystemManager.hpp>
#include <robot2D/Ecs/Scene.hpp>

namespace robot2D::ecs {

    namespace {
        constexpr unsigned maxComponentsContainerValue = 64;
    }

    EntityManager::EntityManager(ComponentManager& componentManager, Scene* scene): m_entityCounter(0),
    m_componentManager(componentManager),
    m_componentContainers(maxComponentsContainerValue),
    m_componentMasks(),
    m_ownerScene{scene}
    {}

    Entity EntityManager::createEntity() {
        Entity entity{this, m_entityCounter};
        if(m_entityCounter >= m_destroyFlags.size())
            m_destroyFlags.resize(m_destroyFlags.size() + 1000);
        m_entityCounter++;
        return entity;
    }


    Bitmask EntityManager::getComponentBitmask(Entity entity) {
        const auto index = entity.getIndex();
     //   assert(index < m_componentMasks.size());
        return m_componentMasks[index];
    }

    bool EntityManager::removeEntity(Entity entity) {
        for(auto& container: m_componentContainers) {
            if(container)
                container -> removeEntity(entity.getIndex());
        }

        bool maskDeleted = static_cast<bool>(m_componentMasks.erase(entity.getIndex()));
        return maskDeleted;
    }

    bool EntityManager::entityDestroyed(Entity entity) {
        return m_destroyFlags[entity.m_id];
    }

    void EntityManager::markDestroyed(Entity entity) {
        m_destroyFlags[entity.m_id] = true;
    }

    void EntityManager::removeEntityFromScene(Entity entity) {
        if(!m_ownerScene)
            return;
        m_ownerScene -> removeEntity(entity);
    }

}