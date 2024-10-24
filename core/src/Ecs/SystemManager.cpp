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

#include <robot2D/Ecs/SystemManager.hpp>
#include <robot2D/Ecs/EntityManager.hpp>
#include <robot2D/Ecs/Scene.hpp>

namespace robot2D::ecs {
    SystemManager::SystemManager(
            robot2D::MessageBus& messageBus,
            ComponentManager& componentManager,
            Scene* scene
    ):
        m_messageBus(messageBus), m_componentManager(componentManager),
        m_scene{scene} {}

    void SystemManager::removeEntity(Entity entity) {
        for(auto& system: m_systems)
            system -> removeEntity(entity);
    }

    void SystemManager::handleMessage(const robot2D::Message& message) {
        for(auto& system: m_systems)
            system -> onMessage(message);
    }

    void SystemManager::update(float dt) {
        for(auto& system: m_systems)
            system -> update(dt);
    }

    void SystemManager::addEntity(Entity entity) {
        const auto mask = entity.getComponentMask();
        for(auto& system: m_systems) {
            if(system -> fitsRequirements(mask) && !system -> hasEntity(entity)) {
                system -> addEntity(entity);
            }
        }
    }

    bool SystemManager::cloneSelf(Scene* cloneScene, SystemManager& clone, const std::vector<Entity>& newEntities) {
        for(auto& system: m_systems) {
            auto clonedSystem = system -> cloneSelf(cloneScene, newEntities);
            if(auto drawable = std::dynamic_pointer_cast<robot2D::Drawable>(clonedSystem)) {
                cloneScene -> m_drawables.emplace_back(drawable.get());
            }

            if(!clonedSystem)
                continue;
            clone.m_systems.emplace_back(clonedSystem);
        }

        return true;
    }

    bool SystemManager::clearSelf() {
        for(auto& system: m_systems)
            system.reset();
        m_systems.clear();
        return true;
    }

}