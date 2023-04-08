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

#include <editor/Scene.hpp>
#include <editor/Components.hpp>

#include <editor/RendererSystem.hpp>
#include <editor/TextSystem.hpp>

#include <editor/scripting/ScriptingEngine.hpp>

namespace editor {

    Scene::Scene(robot2D::MessageBus& messageBus):
    m_scene(messageBus),
    m_messageBus{messageBus}
    {
        initScene();
    }

    void Scene::initScene() {
        m_scene.addSystem<RenderSystem>(m_messageBus);
        m_scene.addSystem<TextSystem>(m_messageBus);
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
        for(auto& entity: m_sceneEntities) {
            if(entity.hasComponent<ScriptComponent>())
                ScriptEngine::onUpdateEntity(entity, dt);
            auto& ts = entity.getComponent<TransformComponent>();
            if(ts.hasChildren()) {
                for(auto& child: ts.getChildren()) {
                    if(child.hasComponent<ScriptComponent>())
                        ScriptEngine::onUpdateEntity(child, dt);;
                }
            }
        }

        m_physicsAdapter -> update(dt);
    }

    void Scene::addEmptyEntity() {
        auto entity = m_scene.createEntity();
        entity.addComponent<IDComponent>(UUID());

        entity.addComponent<TagComponent>();
        /*
            auto& transform = entity.addComponent<Transform3DComponent>();
            transform.setPosition({0.F, 0.F, 0.F});
            transform.setScale({1.F, 1.F, 0.F});
         */
        auto& transform = entity.addComponent<TransformComponent>();
        transform.setPosition({100.F, 100.F});
        transform.setScale({ 20.F, 20.F });
        entity.addComponent<DrawableComponent>();

        m_sceneEntities.emplace_back(entity);
    }

    robot2D::ecs::Entity Scene::createEntity() {
        auto entity = m_scene.createEntity();
        return entity;
    }

    void Scene::removeEntity(robot2D::ecs::Entity entity) {
        auto found = std::find_if(m_sceneEntities.begin(),
                                  m_sceneEntities.end(),
                                  [&entity](robot2D::ecs::Entity ent) {
            return ent.getIndex() == entity.getIndex();
        });
        if(found == m_sceneEntities.end())
            return;

        auto& ts = found -> getComponent<TransformComponent>();
        for(auto child: ts.getChildren())
            child.removeSelf();

        m_scene.removeEntity(entity);
        m_sceneEntities.erase(found, m_sceneEntities.end());
    }

    void Scene::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
        target.draw(m_scene);
    }

    void Scene::onRuntimeStart() {
        m_running = true;
        onPhysics2DRun();

        m_scene.getSystem<RenderSystem>() -> setScene(this);

        ScriptEngine::onRuntimeStart(this);
        for(auto& entity: m_sceneEntities) {
            if(entity.hasComponent<ScriptComponent>())
                ScriptEngine::onCreateEntity(entity);
            auto& ts = entity.getComponent<TransformComponent>();
            if(ts.hasChildren()) {
                for(auto& child: ts.getChildren()) {
                    if(child.hasComponent<ScriptComponent>())
                        ScriptEngine::onCreateEntity(child);
                }
            }
        }

    }

    void Scene::onRuntimeStop() {
        m_scene.getSystem<RenderSystem>() -> setScene(nullptr);
        m_running = false;
        onPhysics2DStop();
        ScriptEngine::onRuntimeStop();
    }

    void Scene::onPhysics2DRun() {
        m_physicsAdapter = getPhysics2DAdapter(PhysicsAdapterType::Box2D);
        m_physicsAdapter -> start(m_sceneEntities);

        m_physicsAdapter -> registerCallback(PhysicsCallbackType::Enter, [](const Physics2DContact&) {
            ScriptEngine::onCollision2DBegin();
        });

        m_physicsAdapter -> registerCallback(PhysicsCallbackType::Exit, [](const Physics2DContact&) {
            ScriptEngine::onCollision2DEnd();
        });
    }

    void Scene::onPhysics2DStop() {
        m_physicsAdapter -> stop();
        m_physicsAdapter.reset();
    }

    void Scene::addAssociatedEntity(robot2D::ecs::Entity entity) {
        m_sceneEntities.emplace_back(entity);
    }

    void Scene::removeEntityChild(robot2D::ecs::Entity entity) {
        m_sceneEntities.erase(std::remove_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                                             [&entity](robot2D::ecs::Entity child) {
            return entity.getIndex() == child.getIndex();
        }), m_sceneEntities.end());
    }

}

