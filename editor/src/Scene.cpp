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

#include <editor/Scene.hpp>
#include <editor/Components.hpp>
#include <editor/Systems.hpp>
#include <robot2D/Graphics/RenderTarget.hpp>

namespace editor {
    Scene::Scene(robot2D::MessageBus& messageBus): m_scene(messageBus, false),
    m_messageBus{messageBus} {
        initScene();
    }

    void Scene::initScene() {
        //todo addBaseSystems to Work
        m_scene.addSystem<RenderSystem>(m_messageBus);
    }

    robot2D::ecs::EntityList& Scene::getEntities() {
        return m_sceneEntities;
    }

    const robot2D::ecs::EntityList& Scene::getEntities() const {
        return m_sceneEntities;
    }

    void Scene::update(float dt) {
        m_scene.update(dt);
    }

    void Scene::updateRuntime(float dt) {

    }

    void Scene::addEmptyEntity() {
        auto entity = m_scene.createEntity();
        entity.addComponent<TagComponent>();
        auto& transform = entity.addComponent<TransformComponent>();
        transform.setPosition({100.F, 100.F});
        transform.setScale({100.F, 100.F});
        m_sceneEntities.emplace_back(entity);
    }

    robot2D::ecs::Entity Scene::createEntity() {
        auto entity = m_scene.createEntity();
        m_sceneEntities.emplace_back(entity);
        return m_sceneEntities.back();
    }

    void Scene::removeEntity(robot2D::ecs::Entity entity) {
        auto found = std::find_if(m_sceneEntities.begin(), m_sceneEntities.end(), [&entity](robot2D::ecs::Entity it) {
            return it == entity;
        });
        if(found == m_sceneEntities.end())
            return;
        m_sceneEntities.erase(found);
        m_scene.removeEntity(entity);
    }

    void Scene::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
        target.draw(m_scene);
    }



}

