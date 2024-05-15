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

#include <robot2D/Core/Assert.hpp>
#include <robot2D/Util/Logger.hpp>
#include <robot2D/Ecs/EntityManager.hpp>
#include <robot2D/Ecs/Scene.hpp>

namespace robot2D::ecs {

    namespace {
        constexpr unsigned maxComponentsContainerValue = 64;
    }

    CustomDestroyComponent::~CustomDestroyComponent() = default;

    EntityManager::EntityManager(ComponentManager& componentManager, Scene* scene): m_entityCounter(0),
    m_componentManager(componentManager),
    m_componentContainers(maxComponentsContainerValue),
    m_componentContainersDeleteBuffer(maxComponentsContainerValue),
    m_componentMasks(),
    m_ownerScene{scene}
    {}

    Entity EntityManager::createEntity(bool needAddToScene) {
        Entity entity{this, m_entityCounter};
        m_destroyFlags[m_entityCounter++] = false;
        entity.m_needAddToScene = needAddToScene;
        return entity;
    }


    Bitmask EntityManager::getComponentBitmask(Entity entity) {
        const auto index = entity.getIndex();
        return m_componentMasks[index];
    }

    bool EntityManager::removeEntity(Entity entity) {
        for(auto& container: m_componentContainers) {
            if(!container)
                continue;
            const auto componentID = container -> getID();
            Bitmask bitmask;
            bitmask.turnOnBit(componentID);
            auto entityMask = entity.getComponentMask();

            if(!bitmask.matches(entityMask, bitmask.getBitset()))
                continue;
            if(m_componentContainersDeleteBuffer[componentID] == nullptr) {
                m_componentContainersDeleteBuffer[componentID] = container -> cloneEmpty();
            }
            if (!container -> cloneTo(m_componentContainersDeleteBuffer[componentID], entity.getIndex())) {
                RB_CORE_ERROR("EntityManager: Can't clone component, index = {0}", entity.getIndex());
            }
            container -> removeEntity(entity.getIndex());
        }

        bool maskDeleted = static_cast<bool>(m_componentMasks.erase(entity.getIndex()));
        return maskDeleted;
    }

    bool EntityManager::entityDestroyed(Entity entity) {
/*        if(entity.m_id >= m_destroyFlags.size())
            return true;*/
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

    void EntityManager::addEntityToScene(robot2D::ecs::Entity entity) {
        if(entity.m_needAddToScene)
            m_ownerScene -> addEntity(entity);
    }

    Entity EntityManager::duplicateEntity(robot2D::ecs::Entity entity) {
        auto duplicated = createEntity();
        for(auto& componentContainer: m_componentContainers) {
            if(!componentContainer)
                continue;
            if(componentContainer -> hasEntity(entity.getIndex()))
                componentContainer -> duplicate(entity.getIndex(), duplicated.getIndex());
        }
        m_componentMasks[duplicated.getIndex()].turnOnBits(entity.getComponentMask().getBitset());

        return duplicated;
    }

    bool EntityManager::restoreEntity(Entity entity) {
        for(auto& container: m_componentContainersDeleteBuffer) {
            if(!container)
                continue;
            const auto componentID = container -> getID();
            if(!container -> hasEntity(entity.getIndex()))
                continue;
            if (!container -> cloneTo(m_componentContainers[componentID], entity.getIndex())) {
                RB_CORE_ERROR("EntityManager: Can't clone component, index = {0}", entity.getIndex());
                return false;
            }
            m_componentMasks[entity.getIndex()].turnOnBit(componentID);
            container -> removeEntity(entity.getIndex());
        }

        if(entity.getIndex() < m_destroyFlags.size())
            m_destroyFlags[entity.getIndex()] = false;
        else
            return false;
        
        return true;
    }

    bool EntityManager::cloneSelf(EntityManager& cloneManager) {

        /// \brief return true if id in destroyFlags else false
        const auto filterEntityFunction = [this](EntityID id) {
            return (m_destroyFlags.find(id) != m_destroyFlags.end());
        };

        /// \brief Clone ComponentContainers: deep copy of containers and copy only not destroyed entities
        {
            int index = -1;
            auto& componentContainers = cloneManager.m_componentContainers;
            for(auto& container: m_componentContainers) {
                ++index;
                if(!container)
                    continue;

                if(!componentContainers[index])
                    componentContainers[index] = container -> cloneEmpty();

                if(!container -> cloneSelf(componentContainers[index], filterEntityFunction))
                    return false;
            }
        }

        {
            auto& componentMasks = cloneManager.m_componentMasks;
            for(const auto& [key, value]: m_componentMasks) {
                if(filterEntityFunction(key))
                    continue;
                componentMasks[key] = value;
            }
        }


        /// FixMe(a.raag): entity is invalid because original scene can have destroyed entities
        return true;
    }

    bool EntityManager::clearSelf() {
        m_entityCounter = 0;
        m_destroyFlags.clear();
        m_componentContainers.clear();
        m_componentContainersDeleteBuffer.clear();
        m_componentMasks.clear();
        return true;
    }

} // namespace robot2D::ecs