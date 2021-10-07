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

#include <robot2D/Ecs/EntityManager.hpp>
#include <robot2D/Ecs/SystemManager.hpp>

namespace robot2D::ecs {

    EntityManager::EntityManager(ComponentManager& componentManager): m_entityCounter(0),
    m_componentManager(componentManager),
    m_componentContainers(64),
    m_componentMasks() {
        m_componentMasks.resize(50);
    }

    Entity EntityManager::createEntity() {
        Entity entity{this, m_entityCounter};
        m_entityCounter++;
        return entity;
    }


    Bitmask EntityManager::getComponentBitmask(Entity entity) {
        const auto index = entity.getIndex();
        assert(index < m_componentMasks.size());
        return m_componentMasks[index];
    }

    // don't remove :)
    bool EntityManager::removeEntity(Entity entity) {
        (void)entity;
        return true;
    }


}