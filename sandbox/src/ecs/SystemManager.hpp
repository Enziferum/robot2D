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

    class SystemManager {
    public:
        SystemManager(robot2D::MessageBus& messageBus, ComponentManager& m_componentManager);
        ~SystemManager() = default;

        template<class T, typename ...Args>
        T& addSystem(Args&& ...args);

        template<typename T>
        T* getSystem();

        void addEntity(Entity entity);
        void removeEntity(Entity entity);

        void handleMessage(const robot2D::Message& message);
        void update(float dt);
    private:
        robot2D::MessageBus& m_messageBus;
        std::vector<System::Ptr> m_systems;
        ComponentManager& m_componentManager;
    };


    template<class T, typename... Args>
    T& SystemManager::addSystem(Args&& ... args) {
        UniqueType systemId(typeid(T));
        auto it = std::find_if(m_systems.begin(), m_systems.end(), [&systemId](const System::Ptr& ptr ) {
            return ptr->m_systemId == systemId;
        });

        if(it != m_systems.end()) {
            return *(dynamic_cast<T*>(it->get()));
        }

        auto& system = m_systems.emplace_back(std::make_shared<T>(std::forward<Args>(args)...));
        system -> processRequirements(m_componentManager);
        return *(dynamic_cast<T*>(m_systems.back().get()));
    }

    template<typename T>
    T* SystemManager::getSystem() {
        UniqueType systemId(typeid(T));
        auto it = std::find_if(m_systems.begin(), m_systems.end(), [&systemId](const System::Ptr& ptr ) {
            return ptr->m_systemId == systemId;
        });
        return dynamic_cast<T*>(it->get());
    }

}
