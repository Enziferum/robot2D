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

#include <algorithm>
#include "System.hpp"

namespace ecs {
    System::System(const SystemID& systemId, SystemManager* systemManager, robot2D::MessageBus& messageBus):
    m_systemManager(systemManager), m_messageBus(messageBus), m_systemId(systemId) {

    }

    System::~System() {}

    bool System::fitsRequirements(const Bitmask& bitmask) {
        auto it = std::find_if(m_requirements.begin(), m_requirements.end(), [&bitmask](Bitmask& mask) {
            return mask.matches(bitmask, mask.getBitset());
        });
        return it != m_requirements.end();
    }

    bool System::addEntity(const Entity& entityId) {
        if(hasEntity(entityId))
            return false;
        m_entities.emplace_back(entityId);
        return true;
    }

    bool System::hasEntity(const Entity& entityId) {
        auto it = std::find_if(m_entities.begin(), m_entities.end(),
                               [&entityId](const Entity& id) {
                                   return entityId == id;
                               });
        return it != m_entities.end();
    }

    bool System::removeEntity(const Entity& entityId) {
        auto it = std::find_if(m_entities.begin(), m_entities.end(),
                               [&entityId](const Entity& id) {
            return entityId == id;
        });

        if(it == m_entities.end())
            return false;
        m_entities.erase(it);
        return true;
    }

}