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

#include <robot2D/Graphics/RenderTarget.hpp>

#include <editor/Scene.hpp>
#include <editor/Components.hpp>

#include <editor/RendererSystem.hpp>
#include <editor/TextSystem.hpp>

#include <editor/scripting/ScriptingEngine.hpp>
#include <editor/panels/TreeHierarchy.hpp>
#include <editor/Messages.hpp>

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

//        auto entity = m_scene.createEntity();
//        entity.addComponent<IDComponent>(UUID());
//        entity.addComponent<TagComponent>().setTag("MainCamera");
//
//        auto& transform = entity.addComponent<TransformComponent>();
//        transform.setPosition({100.F, 100.F});
//        transform.setScale({ 20.F, 20.F });
//        entity.addComponent<DrawableComponent>();
//
//        auto& cameraComponent = entity.addComponent<CameraComponent>();
//        /// todo create camera component
//
//        m_sceneEntities.emplace_back(entity);
    }

    robot2D::ecs::EntityList& Scene::getEntities() {
        return m_sceneEntities;
    }

    const robot2D::ecs::EntityList& Scene::getEntities() const {
        return m_sceneEntities;
    }

    void Scene::update(float dt) {
        m_deletePendingEntities.swap(m_deletePendingBuffer);
        for(auto entity: m_deletePendingEntities) {
            auto& ts = entity.getComponent<TransformComponent>();
            for(auto child: ts.getChildren())
                child.removeSelf();

            m_scene.removeEntity(entity);

            m_sceneEntities.erase(std::find_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                                                              [&entity](robot2D::ecs::Entity ent) {
                                                                  return ent.getIndex() == entity.getIndex();
            }), m_sceneEntities.end());
        }
        if(m_onDeleteFinishCallback && !m_deletePendingEntities.empty())
            m_onDeleteFinishCallback();

        m_deletePendingEntities.clear();

        for(auto item: m_insertItems) {
            auto source = std::get<1>(item);
            m_sceneEntities.insert(std::get<0>(item), source);

            if(std::get<2>(item) == ReorderDeleteType::Last) {
                auto found = util::find_last_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                                                [&source](robot2D::ecs::Entity item) {
                                                    return item.getIndex() == source.getIndex();
                                                });
                m_sceneEntities.erase(found);
            }
            else if(std::get<2>(item) == ReorderDeleteType::First) {
                auto found = util::find_first_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                                                 [&source](robot2D::ecs::Entity item) {
                    return item.getIndex() == source.getIndex();
                });
                m_sceneEntities.erase(found);
            }
        }
        m_insertItems.clear();

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
        m_hasChanges = true;
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
        m_deletePendingBuffer.emplace_back(*found);
        m_hasChanges = true;
    }

    void Scene::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
        target.draw(m_scene);
    }

    void Scene::onRuntimeStart(ScriptInteractor::Ptr scriptInteractor) {
        m_running = true;
        onPhysics2DRun();

        m_scene.getSystem<RenderSystem>() -> setScene(this);

        ScriptEngine::onRuntimeStart(scriptInteractor);
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
        m_hasChanges = true;
    }

    void Scene::setRuntimeCamera(bool flag) {
        m_scene.getSystem<RenderSystem>() -> setRuntimeFlag(flag);
    }

    void Scene::setBefore(robot2D::ecs::Entity source, robot2D::ecs::Entity target) {
        m_hasChanges = true;

        m_scene.getSystem<RenderSystem>() -> setBefore(source, target);

        auto sourceIter = std::find_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                                       [&source](robot2D::ecs::Entity item) {
            return item.getIndex() == source.getIndex();
        });

        auto targetIter = std::find_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                                       [&target](robot2D::ecs::Entity item) {
            return item.getIndex() == target.getIndex();
        });

        auto sourceOldDistance = std::distance(m_sceneEntities.begin(), sourceIter);
        auto targetOldDistance = std::distance(m_sceneEntities.begin(), targetIter);

        if(sourceOldDistance > targetOldDistance) {
            /// insert before remove last
            m_insertItems.push_back(std::make_tuple(targetIter, source, ReorderDeleteType::Last));
        }
        else if (sourceOldDistance < targetOldDistance) {
            m_insertItems.push_back(std::make_tuple(targetIter, source, ReorderDeleteType::First));
        }
    }

    robot2D::ecs::Entity Scene::duplicateEntity(robot2D::vec2f mousePos, robot2D::ecs::Entity entity) {
        auto dupEntity = m_scene.duplicateEntity(entity);
        std::string name = entity.getComponent<TagComponent>().getTag();
        name += "(Clone)";
        dupEntity.getComponent<IDComponent>().ID = UUID();
        dupEntity.getComponent<TagComponent>().setTag(name);
        dupEntity.getComponent<TransformComponent>().setPosition(mousePos);

        auto* msg = m_messageBus.postMessage<EntityDuplication>(MessageID::EntityDuplicate);
        msg -> entityID = dupEntity.getComponent<IDComponent>().ID;
        m_sceneEntities.emplace_back(dupEntity);

        return dupEntity;
    }

    void Scene::registerOnDeleteFinish(std::function<void()>&& callback) {
        m_onDeleteFinishCallback = std::move(callback);
    }

    robot2D::ecs::Entity Scene::createEmptyEntity() {
        return m_scene.createEmptyEntity();
    }

    robot2D::ecs::Entity Scene::duplicateEmptyEntity(robot2D::ecs::Entity entity) {
        auto dupEntity = m_scene.duplicateEntity(entity);
        dupEntity.getComponent<IDComponent>().ID = UUID();
        m_sceneEntities.emplace_back(dupEntity);
        return dupEntity;
    }

    void Scene::removeDuplicate(robot2D::ecs::Entity entity) {


        auto found = std::find_if(m_sceneEntities.begin(),
                                  m_sceneEntities.end(),
                                  [&entity](robot2D::ecs::Entity ent) {
                                      return ent.getIndex() == entity.getIndex();
                                  });
        if(found == m_sceneEntities.end())
            return;
        m_deletePendingBuffer.emplace_back(*found);
        m_hasChanges = true;

        m_scene.removeEntity(entity);
    }

    DeletedEntitiesRestoreInformation Scene::removeEntities(std::vector<robot2D::ecs::Entity>& removingEntities) {
        DeletedEntitiesRestoreInformation restoreInformation;
        restoreInformation.anchorEntitiesUuids.reserve(removingEntities.size());

        int offsetIndex = 0;
        for(auto iter = m_sceneEntities.begin(); iter < m_sceneEntities.end(); ++iter) {
            auto entity = *iter;

            /// TODO(a.raag): check removing and all entities
            if(entity == removingEntities[offsetIndex]) {
                if(iter != m_sceneEntities.begin()) {
                    auto prevIter = iter - 1;
                    auto prevEntity = (*prevIter);
                    restoreInformation.push(prevEntity, removingEntities[offsetIndex], false, false);
                }
                else
                    restoreInformation.push(entity, removingEntities[offsetIndex], true, false);

                //m_scene.removeEntity(removingEntities[offsetIndex]);
                offsetIndex++;
            }

            if(entity.getComponent<TransformComponent>().hasChildren()) {
//                if(iter != m_sceneEntities.begin()) {
//                    auto prevIter = iter - 1;
//                    auto prevEntity = (*prevIter);
//                    restoreInformation.push(prevEntity, false, true);
//                }
//                else
//                    restoreInformation.push(entity, true, true);
            }
        }


        for(auto& ent: removingEntities)
            m_deletePendingBuffer.push_back(ent);

//        m_sceneEntities.erase(std::remove_if(m_sceneEntities.begin(), m_sceneEntities.end(),
//                                             [&removingEntities](auto& ent) {
//            return std::find(removingEntities.begin(), removingEntities.end(), ent) != removingEntities.end();
//        }), m_sceneEntities.end());

        return restoreInformation;
    }

    void Scene::restoreEntities(DeletedEntitiesRestoreInformation& restoreInformation) {

        for(auto& obj: restoreInformation.anchorEntitiesUuids) {
            if(obj.child) {
                auto found = std::find_if(m_sceneEntities.begin(),
                                          m_sceneEntities.end(), [this, obj](robot2D::ecs::Entity ent) {
                   return ent == obj.entity;
                });
                if(found != m_sceneEntities.end()) {
                    auto& transform = (*found).getComponent<TransformComponent>();
                }
            }
            else {
                m_scene.restoreEntity(obj.entity);

                if(obj.first) {
                    m_sceneEntities.insert(m_sceneEntities.begin(), obj.entity);
                }
                else {
                    auto prevFound = std::find_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                                                  [&obj](robot2D::ecs::Entity entity) {
                       return obj.anchorEntity == entity;
                    });

                    m_sceneEntities.insert(prevFound + 1, obj.entity);
                }
            }
        }
    }

}

