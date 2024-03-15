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
#include <vector>
#include <algorithm>
#include <unordered_map>

#include <robot2D/Config.hpp>
#include <robot2D/Core/MessageBus.hpp>
#include "System.hpp"

namespace robot2D::ecs {

    class Scene;
    class ROBOT2D_EXPORT_API SystemManager {
    public:
        SystemManager(robot2D::MessageBus& messageBus,
                      ComponentManager& m_componentManager,
                      Scene* scene);
        SystemManager(const SystemManager& other) = delete;
        SystemManager& operator=(const SystemManager& other) = delete;
        SystemManager(SystemManager&& other) = delete;
        SystemManager& operator=(SystemManager&& other) = delete;
        ~SystemManager() = default;

        template<typename T, typename ...Args>
        T& addSystem(Args&& ...args);

        template<typename T>
        T* getSystem();

        template<typename T>
        const T* getSystem() const;

        template<typename T>
        bool hasSystem() const;

        void addEntity(Entity entity);
        void removeEntity(Entity entity);

        void handleMessage(const robot2D::Message& message);
        void update(float dt);
    private:
        robot2D::MessageBus& m_messageBus;
        std::vector<System::Ptr> m_systems;
        ComponentManager& m_componentManager;

        Scene* m_scene;
    };


    template<class T, typename... Args>
    T& SystemManager::addSystem(Args&& ... args) {
        static_assert(std::is_base_of_v<System, T> && "Possible add only systems");

        UniqueType systemId(typeid(T));
        auto found = std::find_if(m_systems.begin(), m_systems.end(), [&systemId](const System::Ptr& ptr ) {
            return ptr -> m_systemId == systemId;
        });

        if(found != m_systems.end()) {
            return *(dynamic_cast<T*>(found -> get()));
        }

        auto& system = m_systems.emplace_back(std::make_shared<T>(std::forward<Args>(args)...));
        system -> setScene(m_scene);
        system -> processRequirements(m_componentManager);

        return *(dynamic_cast<T*>(m_systems.back().get()));
    }

    template<typename T>
    T* SystemManager::getSystem() {
        UniqueType systemId(typeid(T));

        auto found = std::find_if(m_systems.begin(), m_systems.end(), [&systemId](const System::Ptr& ptr ) {
            return ptr->m_systemId == systemId;
        });

        if(found == m_systems.end())
            return nullptr;

        return dynamic_cast<T*>(found -> get());
    }

    template<typename T>
    const T* SystemManager::getSystem() const {
        UniqueType systemId(typeid(T));

        auto found = std::find_if(m_systems.begin(), m_systems.end(), [&systemId](const System::Ptr& ptr ) {
            return ptr -> m_systemId == systemId;
        });

        if(found == m_systems.end())
            return nullptr;

        return dynamic_cast<T*>(found -> get());
    }

    template<typename T>
    bool SystemManager::hasSystem() const {
        UniqueType systemId(typeid(T));
        auto found = std::find_if(m_systems.begin(), m_systems.end(), [&systemId](const System::Ptr& ptr ) {
            return ptr -> m_systemId == systemId;
        });

        return found != m_systems.end();
    }

}
