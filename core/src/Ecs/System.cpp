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
#include <robot2D/Ecs/System.hpp>

namespace robot2D::ecs {
    System::System(robot2D::MessageBus& messageBus, UniqueType uniqueType):
    m_systemId(uniqueType),
    m_messageBus(messageBus) {}

    System::~System() {}

    bool System::fitsRequirements(Bitmask bitmask) {
        if(bitmask.matches(m_mask, m_mask.getBitset()))
            return true;
        return false;
    }

    bool System::addEntity(Entity entity) {
        if(hasEntity(entity))
            return false;
        m_entities.emplace_back(entity);
        onEntityAdded(entity);
        return true;
    }

    bool System::hasEntity(Entity entity) {
        auto found = std::find_if(m_entities.begin(), m_entities.end(),
                               [&entity](const Entity& id) {
                                   return entity == id;
                               });
        return found != m_entities.end();
    }

    void System::setScene(Scene* scene) {
        m_scene = scene;
    }

    Scene* System::getScene() {
        return m_scene;
    }

    bool System::removeEntity(Entity entity) {
        auto found = std::find_if(m_entities.begin(), m_entities.end(),
                               [&entity](const Entity& containerEntity) {
            return entity == containerEntity;
        });

        if(found == m_entities.end())
            return false;
        onEntityRemoved(*found);
        m_entities.erase(found);
        return true;
    }

    void System::onMessage([[maybe_unused]] const robot2D::Message& message) {}

    void System::update([[maybe_unused]] float dt) {}

    void System::onEntityAdded([[maybe_unused]] Entity entity) {}

    void System::onEntityRemoved([[maybe_unused]] Entity entity) {}

    void System::processRequirements(ComponentManager& componentManager) {
        for(auto& type: m_pendingTypes) {
            auto index = componentManager.getIDFromIndex(type);
            m_mask.turnOnBit(index);
        }
        m_pendingTypes.clear();
    }

    bool System::cloneBase(System::Ptr clonedSystem, Scene* scene, const std::vector<Entity>& newEntities) {
        for(const auto& entity: newEntities) {
            auto mask = entity.getComponentMask();
            if(fitsRequirements(mask))
                clonedSystem -> addEntity(entity);
        }

        clonedSystem -> m_mask = m_mask;
        clonedSystem -> m_scene = scene;
        return true;
    }
}