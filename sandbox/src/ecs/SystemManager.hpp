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
#include <vector>
#include <algorithm>
#include <unordered_map>

#include <robot2D/Core/MessageBus.hpp>
#include "System.hpp"
#include "EventQueue.hpp"

namespace ecs {


    class EntityManager;

    class SystemManager {
    public:
        SystemManager(robot2D::MessageBus& messageBus);
        ~SystemManager() = default;


        template<typename T>
        bool registerSystem(const SystemID& systemId);

        template<typename T>
        T* getSystem(const SystemID& systemId);

        void entityModified(const Entity& entityId, const Bitmask& entityMask);
        bool removeEntity(const Entity& entity);

        void handleMessage(const robot2D::Message& message);
        void update(float dt);
    private:
        robot2D::MessageBus& m_messageBus;
        std::unordered_map<SystemID, System::Ptr> m_systems;
    };
    using SystemPair = std::pair<SystemID, System::Ptr>;

    template<typename T>
    bool SystemManager::registerSystem(const SystemID& systemId) {

        auto it = std::find_if(m_systems.begin(), m_systems.end(), [&systemId](const SystemPair& pair ) {
            return pair.first == systemId;
        });
        if (it != m_systems.end())
            return false;

        m_systems[systemId] = std::make_shared<T>(this, m_messageBus);
        if (m_systems[systemId] == nullptr)
            return false;
        return true;
    }

    template<typename T>
    T* SystemManager::getSystem(const SystemID& systemId) {
        auto it = m_systems.find(systemId);
        if (it == m_systems.end())
            return nullptr;
        return dynamic_cast<T*>(it->second.get());
    }

}
