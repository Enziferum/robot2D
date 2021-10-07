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

#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Ecs/Scene.hpp>
#include "../../../editor/incl/editor/Scene.hpp"


namespace robot2D::ecs {

    Scene::Scene(robot2D::MessageBus& messageBus, const bool& useSystems):
    m_messageBus(messageBus),
    m_componentManager(),
    m_entityManager(m_componentManager),
    m_systemManager(messageBus, m_componentManager),
    m_useSystems(useSystems) {

    }

    Entity Scene::createEntity() {
        Entity entity = m_entityManager.createEntity();
        if(!m_useSystems)
            return entity;
        m_addPending.emplace_back(entity);
        return m_addPending.back();
    }

    void Scene::removeEntity(Entity entity) {
        m_deletePending.emplace_back(entity);
    }

    void Scene::update(float dt) {
        for(auto& entity: m_deletePending) {
            if(m_useSystems)
                m_systemManager.removeEntity(entity);
            m_entityManager.removeEntity(entity);
        }
        m_deletePending.clear();

        for(auto& entity: m_addPending) {
            m_systemManager.addEntity(entity);
        }
        m_deletePending.clear();
        if(m_useSystems)
            m_systemManager.update(dt);
    }

    void Scene::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
        (void)states;
        for(auto& it: m_drawables)
            target.draw(*it);
    }

}

