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

#include "EcsManager.hpp"

namespace ecs {
    EcsManager::EcsManager(robot2D::MessageBus& messageBus):
            m_entityManager(),
            m_systemManager(messageBus)
    {
        m_entityManager.setSystemManager(&m_systemManager);
    }

    const EntityManager& EcsManager::getEntityManager() const {
        return m_entityManager;
    }

    EntityManager& EcsManager::getEntityManager() {
        return m_entityManager;
    }

    const SystemManager& EcsManager::getSystemManager() const {
        return m_systemManager;
    }

    SystemManager& EcsManager::getSystemManager() {
        return m_systemManager;
    }

    Entity EcsManager::createEntity(const Bitmask& bitmask) {
        return m_entityManager.addEntity(bitmask);
    }
}