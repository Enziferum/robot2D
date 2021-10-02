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

#include "SystemManager.hpp"
#include "EntityManager.hpp"

namespace ecs {
    SystemManager::SystemManager(robot2D::MessageBus& messageBus):
        m_messageBus(messageBus) {

    }

    void SystemManager::entityModified(const Entity& entityId, const Bitmask& entityMask) {
        for(auto& it: m_systems) {
            auto system = it.second;
            if(system -> fitsRequirements(entityMask)) {
                system -> addEntity(entityId);
            } else {
                system -> removeEntity(entityId);
            }
        }
    }

    bool SystemManager::removeEntity(const Entity& entity) {
        for(auto& it: m_systems)
            it.second -> removeEntity(entity);
        return true;
    }

    void SystemManager::handleMessage(const robot2D::Message& message) {
        for(auto& it: m_systems)
            it.second -> onMessage(message);
    }

    void SystemManager::update(float dt) {
        for(auto& it: m_systems)
            it.second -> update(dt);
    }

}