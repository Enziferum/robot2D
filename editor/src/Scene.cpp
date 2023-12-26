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
#include <editor/components/UIHitBox.hpp>
#include <editor/components/ButtonComponent.hpp>

#include <editor/RendererSystem.hpp>
#include <editor/TextSystem.hpp>
#include <editor/AnimatorSystem.hpp>
#include <editor/AnimationSystem.hpp>
#include <editor/UISystem.hpp>

#include <editor/scripting/ScriptingEngine.hpp>
#include <editor/panels/TreeHierarchy.hpp>
#include <editor/Messages.hpp>

namespace editor {

    Scene::Scene(robot2D::MessageBus& messageBus):
    m_scene(messageBus),
    m_CloneScene(messageBus),
    m_messageBus{messageBus}
    {
        initScene();
    }

    void Scene::initScene() {
        m_scene.addSystem<RenderSystem>(m_messageBus);
        m_scene.addSystem<TextSystem>(m_messageBus);
        m_scene.addSystem<AnimatorSystem>(m_messageBus);
        m_scene.addSystem<AnimationSystem>(m_messageBus);
        m_scene.addSystem<UISystem>(m_messageBus);
    }

    void Scene::createMainCamera() {
        auto entity = m_scene.createEntity();
        entity.addComponent<IDComponent>(UUID());
        entity.addComponent<TagComponent>().setTag("MainCamera");

        auto& transform = entity.addComponent<TransformComponent>();
        transform.setPosition({100.F, 100.F});
        transform.setScale({ 20.F, 20.F });
        entity.addComponent<DrawableComponent>();

        auto& cameraComponent = entity.addComponent<CameraComponent>();

        m_sceneEntities.emplace_back(entity);
    }


    Scene::EntityList& Scene::getEntities() {
        return m_sceneEntities;
    }

    const Scene::EntityList& Scene::getEntities() const {
        return m_sceneEntities;
    }

    void Scene::handleEventsRuntime(const robot2D::Event& event) {
        m_scene.getSystem<UISystem>() -> handleEvents(event);
    }


    void Scene::update(float dt) {
        for(auto& item: m_setItems) {
            auto iter = std::get<0>(item);
            if(iter == m_sceneEntities.begin())
                m_sceneEntities.insert(iter, std::get<1>(item));
            bool isChained = std::get<2>(item);
            if(!isChained)
                m_sceneEntities.insert(std::next(iter), std::get<1>(item));
            else {
                auto anchor = std::get<3>(item);
                auto prevFound = std::find_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                                              [&anchor](auto item) {
                                                  return item == anchor;
                                              });
                m_sceneEntities.insert(std::next(prevFound), std::get<1>(item));
            }
        }

        m_setItems.clear();

        m_deletePendingEntities.swap(m_deletePendingBuffer);
        for(auto entity: m_deletePendingEntities) {
            auto& ts = entity.getComponent<TransformComponent>();
            for(auto child: ts.getChildren())
                child.removeSelf();

            m_sceneEntities.erase(std::remove_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                                                              [&entity](robot2D::ecs::Entity item) {
                                                                  return item == entity;
            }), m_sceneEntities.end());

            m_scene.removeEntity(entity);
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
                        ScriptEngine::onUpdateEntity(child, dt);
                }
            }
        }

        m_physicsAdapter -> update(dt);
        m_scene.update(dt);
    }

    void Scene::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
        target.draw(m_scene);
    }

    robot2D::ecs::Entity Scene::createEntity() {
        auto entity = m_scene.createEntity();
        return entity;
    }

    robot2D::ecs::Entity Scene::createEmptyEntity() {
        return m_scene.createEmptyEntity();
    }

    void Scene::addEmptyEntity() {
        auto entity = m_scene.createEntity();
        entity.addComponent<IDComponent>(UUID());
        entity.addComponent<TagComponent>();

        auto& transform = entity.addComponent<TransformComponent>();
        transform.setPosition({100.F, 100.F});
        transform.setScale({ 20.F, 20.F });
        entity.addComponent<DrawableComponent>();

        m_sceneEntities.emplace_back(entity);
        m_hasChanges = true;
    }

    void Scene::addAssociatedEntity(robot2D::ecs::Entity entity) {
        m_sceneEntities.emplace_back(entity);
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


    void Scene::onRuntimeStart(ScriptInteractor::Ptr scriptInteractor) {
        m_running = true;
        m_scene.cloneSelf(m_CloneScene);
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

            if(entity.hasComponent<ButtonComponent>()) {
                auto& hitbox = entity.getComponent<UIHitbox>();
                // TODO(a.raag) ts.getGlobalBounds();
                hitbox.m_area = robot2D::FloatRect::create(ts.getPosition(), ts.getScale() + ts.getPosition());
            }
        }

    }

    void Scene::onRuntimeStop() {
        m_scene.getSystem<RenderSystem>() -> setScene(nullptr);
        m_running = false;
        onPhysics2DStop();
        ScriptEngine::onRuntimeStop();

        m_scene.restoreFromClone(m_CloneScene);
        m_CloneScene.clearAll();
    }

    void Scene::onPhysics2DRun() {
        m_physicsAdapter = getPhysics2DAdapter(PhysicsAdapterType::Box2D);
        m_physicsAdapter -> start(m_sceneEntities);


        m_physicsAdapter -> registerCallback(PhysicsCallbackType::Enter,
                                             [](const Physics2DContact& contact) {
            ScriptEngine::onCollision2DBegin(contact);
        });

        m_physicsAdapter -> registerCallback(PhysicsCallbackType::Exit,
                                             [](const Physics2DContact& contact) {
            ScriptEngine::onCollision2DEnd(contact);
        });
    }

    void Scene::onPhysics2DStop() {
        m_physicsAdapter -> stop();
        m_physicsAdapter.reset();
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

    robot2D::ecs::Entity Scene::getByUUID(UUID uuid) {
        auto start = m_sceneEntities.begin();
        for(; start != m_sceneEntities.end(); ++start) {

            auto entity = *start;
            if(entity.getComponent<TransformComponent>().hasChildren()) {
                auto childEntity = getByUUID(entity, uuid);
                if(childEntity && !childEntity.destroyed())
                    return childEntity;
            }
            if(entity.getComponent<IDComponent>().ID == uuid)
                return entity;
        }

        return robot2D::ecs::Entity{};
    }

    robot2D::ecs::Entity Scene::getByUUID(robot2D::ecs::Entity parent, UUID uuid) {
        auto& transform = parent.getComponent<TransformComponent>();
        for(auto& child: transform.getChildren()) {
            if(!child || child.destroyed())
                continue;

            if(child.getComponent<TransformComponent>().hasChildren()) {
                auto childEntity = getByUUID(child, uuid);
                if(childEntity && !childEntity.destroyed())
                    return childEntity;
            }

            if(child.getComponent<IDComponent>().ID == uuid)
                return child;
        }
        return robot2D::ecs::Entity{};
    }

    void Scene::registerOnDeleteFinish(std::function<void()>&& callback) {
        m_onDeleteFinishCallback = std::move(callback);
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

        auto* msg = m_messageBus.postMessage<EntityDuplication>(MessageID::EntityDuplicate);
        msg -> entityID = dupEntity.getComponent<IDComponent>().ID;

        m_sceneEntities.emplace_back(dupEntity);
        m_hasChanges = true;

        if(entity.getComponent<TransformComponent>().hasChildren())
            duplicateEntityChild(dupEntity, entity);

        dupEntity.getComponent<TransformComponent>().setPosition(mousePos);
        return dupEntity;
    }

    void Scene::duplicateEntityChild(robot2D::ecs::Entity parent, robot2D::ecs::Entity dupEntity) {
        auto& parentTransform = dupEntity.getComponent<TransformComponent>();
        auto& dupParentTransform = parent.getComponent<TransformComponent>();
        dupParentTransform.clearChildren();
        for(auto& child: parentTransform.getChildren()) {
            auto dupChild = m_scene.duplicateEntity(child);
            dupParentTransform.addChild(parent, dupChild);
            if(child.getComponent<TransformComponent>().hasChildren())
                duplicateEntityChild(dupChild, child);
        }
    }

    robot2D::ecs::Entity Scene::duplicateEmptyEntity(robot2D::ecs::Entity entity) {
        auto dupEntity = m_scene.duplicateEntity(entity);
        dupEntity.getComponent<IDComponent>().ID = UUID();
        m_sceneEntities.emplace_back(dupEntity);
        return dupEntity;
    }

    void Scene::removeDuplicate(robot2D::ecs::Entity entity) {
        if(entity.getComponent<TransformComponent>().isChild()) {

        }
        else {
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

        m_scene.removeEntity(entity);
    }

    DeletedEntitiesRestoreInformation Scene::removeEntities(std::vector<robot2D::ecs::Entity>& removingEntities) {
        DeletedEntitiesRestoreInformation restoreInformation;
        std::vector<RemoveEntityInfo> removeInfos{removingEntities.size()};
        std::transform(removingEntities.begin(), removingEntities.end(), removeInfos.begin(),
                       [](robot2D::ecs::Entity entity) {
           return RemoveEntityInfo{entity};
        });
        using FoundIterator = std::list<robot2D::ecs::Entity>::iterator;

        m_deletePendingBuffer.clear();
        for(auto item: removeInfos) {
            FoundIterator found = m_sceneEntities.begin();

            bool isParentDel = false;
            for(; found != m_sceneEntities.end(); ++found) {
                auto iterEntity = *found;
                auto& transform = iterEntity.getComponent<TransformComponent>();

                if(iterEntity == item.entity) {
                    item.isDeleted = true;
                    isParentDel = true;
                    iterEntity.getComponent<UIComponent>().setName(nullptr);
                }

                if(transform.hasChildren())
                    removeChildEntities(restoreInformation, removeInfos, iterEntity, isParentDel);

                if(iterEntity == item.entity) {
                    break;
                }
            }

            if(found == m_sceneEntities.end())
                continue;

            m_deletePendingBuffer.push_back(item.entity);

            if(!restoreInformation.hasItems()) {
                if(found == m_sceneEntities.begin()) {
                    restoreInformation.push(robot2D::ecs::Entity{}, item.entity,
                                            true, false, false);
                }
                else {
                    auto prev = std::prev(found);
                    restoreInformation.push(*prev, item.entity,
                                            false, false, false);
                }
           }
            else {
               auto lastInfo = restoreInformation.getLast();
               if(lastInfo.entity == *(std::prev(found))) {
                   restoreInformation.push(lastInfo.entity, *found, false, true, false);
               }
               else {
                   if(found == m_sceneEntities.begin()) {
                       restoreInformation.push(robot2D::ecs::Entity{}, *found, true, false, false);
                   }
                   else {
                       auto prev = std::prev(found);
                       restoreInformation.push(*prev, *found, false, false, false);
                   }
               }
           }
       }

        return restoreInformation;
    }


    void Scene::removeChildEntities(DeletedEntitiesRestoreInformation& information,
                                    std::vector<RemoveEntityInfo>& removingEntities,
                                    robot2D::ecs::Entity parent,
                                    bool isParentDel) {

        for(auto& item: removingEntities) {

            auto& transform = parent.getComponent<TransformComponent>();
            for(auto& child: transform.getChildren()) {
                if(!child || child.destroyed())
                    continue;
                auto& childTransform = child.getComponent<TransformComponent>();

                if(child == item.entity)
                    isParentDel = true;

                if(childTransform.hasChildren())
                    removeChildEntities(information, removingEntities, child, isParentDel);

                if(child == item.entity) {
                    information.push(parent, child, false, true, true);
                    item.isDeleted = true;
                    child.getComponent<UIComponent>().setName(nullptr);
                    m_scene.removeEntity(child);
                }

                if(isParentDel && !item.isDeleted) {
                    information.push(parent, child, false, true, true);
                    child.getComponent<UIComponent>().setName(nullptr);
                    m_scene.removeEntity(child);
                }

            }
        }

    }

    void Scene::restoreEntities(DeletedEntitiesRestoreInformation& restoreInformation) {
        std::vector<std::pair<robot2D::ecs::Entity, robot2D::ecs::Entity>> childBuffer;

        for(auto& info: restoreInformation.getInfos()) {

            if(info.child) {
                if(info.anchorEntity && !info.anchorEntity.destroyed()) {
                    auto parentTransform =
                            const_cast<TransformComponent*>(&info.anchorEntity.getComponent<TransformComponent>());
                    parentTransform -> addChild(info.anchorEntity, info.entity);
                }
                else {
                    childBuffer.emplace_back(std::make_pair(info.anchorEntity, info.entity));
                }

                m_scene.restoreEntity(info.entity);
                info.entity.getComponent<UIComponent>().setName(&info.entity.getComponent<TagComponent>().getTag());
            }
            else {

                if(info.first) {
                    m_scene.restoreEntity(info.entity);
                    info.entity.getComponent<UIComponent>().setName(&info.entity.getComponent<TagComponent>().getTag());
                    bool isChained = false;
                    m_setItems.emplace_back(m_sceneEntities.begin(),
                                                         info.entity, isChained, robot2D::ecs::Entity{});
                }
                else {
                    if(!info.isChained) {
                        if(!m_scene.restoreEntity(info.entity)) {
                            RB_EDITOR_ERROR("EditorScene: Can't Restore entity with index {0}", info.entity.getIndex());
                        }
                        info.entity.getComponent<UIComponent>().setName(&info.entity.getComponent<TagComponent>().getTag());
                        auto anchorFound = std::find_if(m_sceneEntities.begin(),
                                                        m_sceneEntities.end(), [&info](auto item) {
                            return item == info.anchorEntity;
                        });
                        bool isChained = false;
                        m_setItems.emplace_back(anchorFound, info.entity, isChained, robot2D::ecs::Entity{});
                    }
                    else {
                        if(!m_scene.restoreEntity(info.entity)) {
                            RB_EDITOR_ERROR("EditorScene: Can't Restore entity with index {0}", info.entity.getIndex());
                        };
                        info.entity.getComponent<UIComponent>().setName(&info.entity.getComponent<TagComponent>().getTag());
                        bool isChained = true;
                        m_setItems.emplace_back(m_sceneEntities.end(), info.entity, isChained, info.anchorEntity);
                    }
                }

            }

        }

        for(auto& childPair: childBuffer) {

            auto parent = childPair.first;
            auto child = childPair.second;

            parent.getComponent<TransformComponent>().addChild(parent, child);
        }

    }

    robot2D::ecs::Entity Scene::addEmptyButton() {
        auto entity = m_scene.createEntity();
        auto& transform = entity.addComponent<TransformComponent>();
        entity.addComponent<IDComponent>(UUID());
        entity.addComponent<TagComponent>();
        transform.setPosition({100, 100});
        transform.setScale({20, 20});

        entity.addComponent<ButtonComponent>();
        entity.addComponent<UIHitbox>().callbackIDs[UIHitbox::CallbackID::MouseDown] =
                m_scene.getSystem<UISystem>() -> addMousePressedCallback([](robot2D::ecs::Entity entity, std::uint64_t){
                    auto& btnComp = entity.getComponent<ButtonComponent>();
                    if(btnComp.hasEntity()
                            && btnComp.onClickCallback && !btnComp.clickMethodName.empty())
                        btnComp.onClickCallback(btnComp.scriptEntity, btnComp.clickMethodName);
                });

        m_sceneEntities.emplace_back(entity);
        return entity;
    }

    void Scene::setEditorCamera(IEditorCamera::Ptr editorCamera) {
        m_editorCamera = editorCamera;
        m_scene.getSystem<UISystem>() -> setCamera(m_editorCamera);
    }

    void Scene::registerUICallback(robot2D::ecs::Entity uiEntity) {
        uiEntity.addComponent<UIHitbox>().callbackIDs[UIHitbox::CallbackID::MouseDown] =
                m_scene.getSystem<UISystem>() -> addMousePressedCallback([](robot2D::ecs::Entity entity, std::uint64_t){
                    auto& btnComp = entity.getComponent<ButtonComponent>();
                    if(btnComp.hasEntity()
                       && btnComp.onClickCallback && !btnComp.clickMethodName.empty())
                        btnComp.onClickCallback(btnComp.scriptEntity, btnComp.clickMethodName);
                });
    }


}

