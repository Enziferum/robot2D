/*********************************************************************
(c) Alex Raag 2024
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
#include <optional>
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

    namespace {
        constexpr robot2D::vec2f defaultPosition = {100.f, 100.f};
        constexpr robot2D::vec2f defaultSize = {20.f, 20.f};
    }

    template<typename Container, typename T>
    std::optional<typename Container::iterator> find(Container& container, const T& item) {
        auto found = std::find_if(std::begin(container), std::end(container), [&item](const T& containerItem) {
            return item == containerItem;
        });

        if(found != std::end(container))
            return found;

        return std::nullopt;
    }

    Scene::Scene(robot2D::MessageBus &messageBus) :
            m_sceneGraph(messageBus),
            m_runtimeSceneGraph(messageBus),
            m_scene(messageBus),
            m_runtimeScene{messageBus},
            m_messageBus{messageBus} {
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
        transform.setPosition(defaultPosition);
        transform.setSize(defaultSize);
        entity.addComponent<DrawableComponent>();

        auto& cameraComponent = entity.addComponent<CameraComponent>();
        m_sceneGraph.addEntity(SceneEntity{ entity });
    }


    std::list<SceneEntity>& Scene::getEntities() {
        return m_sceneGraph.getEntities();
    }

    const std::list<SceneEntity>& Scene::getEntities() const {
        return m_sceneGraph.getEntities();
    }

    void Scene::handleEventsRuntime(const robot2D::Event& event) {
        m_scene.getSystem<UISystem>() -> handleEvents(event);
    }


    void Scene::update(float dt) {
        auto& sceneEntities = m_sceneGraph.getEntities();
        for (const auto& restoreData: m_restoreItems) {
            if (restoreData.anchorIterator == sceneEntities.begin())
                sceneEntities.insert(restoreData.anchorIterator, restoreData.sourceEntity);
            if (!restoreData.isChained) {
                if (restoreData.anchorIterator != sceneEntities.end())
                    sceneEntities.insert(std::next(restoreData.anchorIterator), restoreData.sourceEntity);
                else
                    sceneEntities.insert(restoreData.anchorIterator, restoreData.sourceEntity);
            } else {
                auto prevFound = find(sceneEntities, restoreData.anchorEntity);
                if(prevFound)
                    sceneEntities.insert(std::next(*prevFound), restoreData.sourceEntity);
            }
        }

        m_restoreItems.clear();
        m_sceneGraph.update(dt, m_scene);
        m_scene.update(dt);
    }

    void Scene::updateRuntime(float dt, IScriptInteractorFrom::Ptr scriptInteractor) {
        auto scriptingEngine = scriptInteractor -> getScriptingEngine();
        if(!scriptingEngine)
            return;

        for (const auto& entity: m_scriptRuntimeContainer)
            scriptingEngine -> onUpdateEntity(entity, dt);

        m_physicsAdapter -> update(dt);
        m_runtimeScene.update(dt);
    }

    void Scene::draw(robot2D::RenderTarget &target, robot2D::RenderStates states) const {
        if(m_running) {
            target.draw(m_runtimeScene);
            return;
        }
        target.draw(m_scene);
    }

    SceneEntity Scene::createEntity() {
        auto entity = m_scene.createEntity();
        return m_sceneGraph.createEntity(std::move(entity));
    }

    SceneEntity Scene::createEmptyEntity() {
        return SceneEntity(std::move(m_scene.createEmptyEntity()));
    }

    void Scene::addAssociatedEntity(SceneEntity &&entity) {
        m_sceneGraph.addEntity(std::move(entity));
        m_hasChanges = true;
    }


    SceneEntity Scene::addEmptyEntity() {
        auto entity = m_scene.createEntity();
        entity.addComponent<IDComponent>(UUID());
        entity.addComponent<TagComponent>();

        auto &transform = entity.addComponent<TransformComponent>();
        transform.setPosition(defaultPosition);
        transform.setSize(defaultSize);
        entity.addComponent<DrawableComponent>();

        m_sceneGraph.addEntity(SceneEntity{entity});
        m_hasChanges = true;
        auto sceneEntity = m_sceneGraph.getEntities().back();
        return sceneEntity;
    }

    void Scene::onRuntimeStart(IScriptInteractorFrom::Ptr scriptInteractor) {
        m_running = true;
        m_scene.cloneSelf(m_runtimeScene, m_runtimeClonedArray, true);



        m_runtimeSceneGraph.m_AllSceneEntitiesMap.clear();
        m_scriptRuntimeContainer.clear();
        for(const auto& entity: m_runtimeClonedArray) {
            SceneEntity newSceneEntity{entity};
            m_runtimeSceneGraph.m_AllSceneEntitiesMap[newSceneEntity.getUUID()] = newSceneEntity;
            if(entity.hasComponent<ScriptComponent>())
                m_scriptRuntimeContainer.push_back(newSceneEntity);
        }

        onPhysics2DRun(scriptInteractor);

        m_runtimeScene.getSystem<RenderSystem>() -> setScene(this);
        auto scriptingEngine = scriptInteractor -> getScriptingEngine();
        if(!scriptingEngine)
            return;

        scriptingEngine -> onRuntimeStart(scriptInteractor);


      /*  m_runtimeSceneGraph.filterEntities<ScriptComponent>(m_scriptRuntimeContainer);*/
        for (const auto& entity: m_scriptRuntimeContainer)
            scriptingEngine -> onCreateEntity(entity);


        m_runtimeSceneGraph.traverseGraph(std::move([](SceneEntity& sceneEntity) {
            if (sceneEntity.hasComponent<ButtonComponent>()) {
                auto& hitbox = sceneEntity.getComponent<UIHitbox>();
                auto& ts = sceneEntity.getComponent<TransformComponent>();
                // TODO(a.raag) ts.getGlobalBounds();
                hitbox.m_area = robot2D::FloatRect::create(ts.getPosition(),
                                                           ts.getSize() + ts.getPosition());
            }
        }));
    }

    void Scene::onRuntimeStop(IScriptInteractorFrom::Ptr scriptInteractor) {
        m_runtimeScene.getSystem<RenderSystem>() -> setScene(nullptr);
        m_running = false;
        onPhysics2DStop();
        auto scriptingEngine = scriptInteractor -> getScriptingEngine();
        if(!scriptingEngine)
            return;
        scriptingEngine -> onRuntimeStop();
        m_runtimeClonedArray.clear();
        m_runtimeScene.clearSelf();
    }

    void Scene::onPhysics2DRun(IScriptInteractorFrom::Ptr scriptInteractor) {
        m_physicsAdapter = getPhysics2DAdapter(PhysicsAdapterType::Box2D);
        m_listPhysics.clear();

        for(const auto& ecsEntity: m_runtimeClonedArray)
            m_listPhysics.push_back(SceneEntity{ecsEntity});

        m_physicsAdapter -> start(m_listPhysics);
        auto scriptingEngine = scriptInteractor -> getScriptingEngine();
        if(!scriptingEngine)
            return;

        m_physicsAdapter -> registerCallback(PhysicsCallbackType::Enter,
                                           [scriptingEngine](const Physics2DContact& contact) {
                                               scriptingEngine -> onCollision2DBegin(contact);
                                           });

        m_physicsAdapter -> registerCallback(PhysicsCallbackType::Exit,
                                           [scriptingEngine](const Physics2DContact& contact) {
                                               scriptingEngine -> onCollision2DEnd(contact);
                                           });

        m_physicsAdapter -> registerCallback(PhysicsCallbackType::EnterTrigger,
                                           [scriptingEngine](const Physics2DContact& contact) {
                                               scriptingEngine -> onCollision2DBeginTrigger(contact);
                                           });

        m_physicsAdapter -> registerCallback(PhysicsCallbackType::ExitTrigger,
                                           [scriptingEngine](const Physics2DContact& contact) {
                                               scriptingEngine -> onCollision2DEndTrigger(contact);
                                           });
    }

    void Scene::onPhysics2DStop() {
        m_physicsAdapter -> stop();
        m_physicsAdapter.reset();
    }


    void Scene::removeEntityChild(SceneEntity entity) {
        m_hasChanges = true;
    }

    void Scene::setRuntimeCamera(bool flag) {
        if(!m_running)
            m_scene.getSystem<RenderSystem>() -> setRuntimeFlag(flag);
        else
            m_runtimeScene.getSystem<RenderSystem>() -> setRuntimeFlag(flag);
    }

    SceneEntity Scene::getEntity(UUID uuid) const {
        if(m_running)
            return m_runtimeSceneGraph.getEntity(uuid);
        return m_sceneGraph.getEntity(uuid);
    }


    bool Scene::setBefore(SceneEntity source, SceneEntity target) {
        if (m_sceneGraph.setBefore(source, target)) {
            m_scene.getSystem<RenderSystem>() -> setBefore(source.getWrappedEntity(),
                                                         target.getWrappedEntity());
            m_hasChanges = true;
            return true;
        }

        return false;
    }

    SceneEntity Scene::duplicateEntity(robot2D::vec2f mousePos, SceneEntity entity) {
        auto dupEntity = m_scene.duplicateEntity(entity.getWrappedEntity());
        std::string name = entity.getComponent<TagComponent>().getTag();
        name += "(Clone)";
        dupEntity.getComponent<IDComponent>().ID = UUID();
        dupEntity.getComponent<TagComponent>().setTag(name);

        auto* msg =
                m_messageBus.postMessage<EntityDuplication>(MessageID::EntityDuplicate);
        msg -> entityID = dupEntity.getComponent<IDComponent>().ID;
        m_hasChanges = true;

        SceneEntity duplicateSceneEntity{std::move(dupEntity)};
        if(entity.isChild()) {
            auto parent = entity.getParent();
            parent -> addChild(duplicateSceneEntity);
        }
        else {
            m_sceneGraph.addEntity(duplicateSceneEntity);
        }

        if (entity.hasChildren())
            duplicateEntityChild(duplicateSceneEntity, entity);

        return duplicateSceneEntity;
    }

    void Scene::duplicateEntityChild(SceneEntity parent, SceneEntity dupEntity) {
        auto& parentTransform = dupEntity.getComponent<TransformComponent>();
        auto& dupParentTransform = parent.getComponent<TransformComponent>();
        dupParentTransform.clearChildren();

        for(auto& child: parentTransform.getChildren()) {
            auto dupChild = m_scene.duplicateEntity(child.getWrappedEntity());
            //dupParentTransform.addChild(parent, dupChild);
           // if(child.getComponent<TransformComponent>().hasChildren())
              //  duplicateEntityChild(dupChild, child);
        }
    }

    SceneEntity Scene::duplicateEmptyEntity(SceneEntity entity) {
        auto dupEntity = m_scene.duplicateEntity(entity.getWrappedEntity());
        dupEntity.getComponent<IDComponent>().ID = UUID();

        /// TODO(a.raag): create or add ??
        return m_sceneGraph.createEntity(std::move(dupEntity));
    }

    void Scene::removeDuplicate(SceneEntity entity) {
        m_hasChanges = true;
        m_sceneGraph.removeEntity(entity);
    }


    DeletedEntitiesRestoreInformation Scene::removeEntities(std::vector<SceneEntity>& removingEntities) {
        DeletedEntitiesRestoreInformation restoreInformation;
        using RestoreInfo = DeletedEntitiesRestoreInformation::RestoreInfo;

        std::vector<RemoveEntityInfo> removeInfos{removingEntities.size()};
        std::transform(removingEntities.begin(), removingEntities.end(), removeInfos.begin(),
                       [](SceneEntity entity) {
                           return RemoveEntityInfo{entity};
                       });
        using FoundIterator = std::list<SceneEntity>::const_iterator;
        const auto& sceneEntities = m_sceneGraph.getEntities();

        for (auto& item: removeInfos) {
            FoundIterator found = sceneEntities.begin();

            bool isParentDel = false;
            for (; found != sceneEntities.end(); ++found) {
                auto iterEntity = *found;
                if (iterEntity == item.entity) {
                    item.isDeleted = true;
                    isParentDel = true;
                    if(iterEntity.hasComponent<UIComponent>())
                        iterEntity.getComponent<UIComponent>().setName(nullptr);
                }

                if (iterEntity.hasChildren())
                    removeChildEntities(restoreInformation, removeInfos, iterEntity, isParentDel);

                if (iterEntity == item.entity) {
                    break;
                }
            }

            if (found == sceneEntities.end())
                continue;


            m_sceneGraph.removeEntity(item.entity);
            RestoreInfo restoreInfo; // first isChained child

            if (!restoreInformation.hasItems()) {
                if (found == sceneEntities.begin()) {
                    restoreInfo.first = true;
                    restoreInfo.entity = item.entity;
                } else {
                    restoreInfo.anchorEntity = *std::prev(found);
                    restoreInfo.entity = item.entity;
                }
            } else {
                auto lastInfo = restoreInformation.getLast();
                if (lastInfo.entity == *(std::prev(found))) {
                    restoreInfo.anchorEntity = lastInfo.entity;
                    restoreInfo.entity = *found;
                    restoreInfo.isChained = true;
                } else {
                    if (found == sceneEntities.begin()) {
                        restoreInfo.entity = *found;
                        restoreInfo.first = true;
                    } else {
                        auto prev = std::prev(found);
                        restoreInfo.anchorEntity = *prev;
                        restoreInfo.entity = *found;
                    }
                }
            }

            restoreInformation.push(std::move(restoreInfo));
        }

        return restoreInformation;
    }


    void Scene::removeChildEntities(DeletedEntitiesRestoreInformation &information,
                                    std::vector<RemoveEntityInfo> &removingEntities,
                                    const SceneEntity &parent,
                                    bool isParentDel) {

        for (auto& item: removingEntities) {
            for (auto child: parent.getChildren()) {
                if (!child)
                    continue;
                auto& childTransform = child.getComponent<TransformComponent>();

                if (child == item.entity)
                    isParentDel = true;

                if (childTransform.hasChildren())
                    removeChildEntities(information, removingEntities, child, isParentDel);

                if (child == item.entity) {
                    information.push(parent, child, false, true, true);
                    item.isDeleted = true;
                    child.getComponent<UIComponent>().setName(nullptr);
                    m_scene.removeEntity(child.getWrappedEntity());
                }

                if (isParentDel && !item.isDeleted) {
                    information.push(parent, child, false, true, true);
                    child.getComponent<UIComponent>().setName(nullptr);
                    m_scene.removeEntity(child.getWrappedEntity());
                }

            }
        }

    }

    void Scene::restoreEntities(DeletedEntitiesRestoreInformation& restoreInformation) {
        using EntityPair = std::pair<SceneEntity, SceneEntity>;
        std::vector<EntityPair> childBuffer;
        childBuffer.reserve(restoreInformation.getInfos().size());
        auto& sceneEntities = m_sceneGraph.getEntities();

        for (auto& info: restoreInformation.getInfos()) {
            if (!m_scene.restoreEntity(info.entity.getWrappedEntity())) {
                RB_EDITOR_ERROR("EditorScene: Can't Restore entity with index {0}",
                                info.entity.getWrappedEntity().getIndex());
                return;
            }
            auto& tag = info.entity.getComponent<TagComponent>().getTag();
            info.entity.getComponent<UIComponent>().setName(&tag);

            if (info.child) {
                if (info.anchorEntity)
                    info.anchorEntity.addChild(info.entity);
                else
                    childBuffer.emplace_back(info.anchorEntity, info.entity);

            } else {

                RestoreData restoreData;
                restoreData.sourceEntity = info.entity;

                if (info.first) {
                    restoreData.anchorIterator = sceneEntities.begin();
                } else {
                    if (!info.isChained) {
                        auto anchorFound = find(sceneEntities, info.anchorEntity);
                        if(anchorFound)
                            restoreData.anchorIterator = *anchorFound;
                    } else {
                        restoreData.isChained = true;
                        restoreData.anchorIterator = sceneEntities.end();
                        restoreData.anchorEntity = info.anchorEntity;
                    }
                }

                m_restoreItems.emplace_back(restoreData);
            }
        }

        for (auto& [parent, child]: childBuffer)
            parent.addChild(child);

    }

    SceneEntity Scene::addEmptyButton() {
        auto entity = m_scene.createEntity();
        auto &transform = entity.addComponent<TransformComponent>();
        entity.addComponent<IDComponent>(UUID());
        entity.addComponent<TagComponent>();

        transform.setPosition(defaultPosition);
        transform.setSize(defaultSize);

        entity.addComponent<ButtonComponent>();
        entity.addComponent<UIHitbox>().callbackIDs[UIHitbox::CallbackID::MouseDown] =
            m_scene.getSystem<UISystem>() -> addMousePressedCallback([](robot2D::ecs::Entity entity, std::uint64_t) {
                auto& btnComp = entity.getComponent<ButtonComponent>();
                if (btnComp.hasEntity() && btnComp.onClickCallback && !btnComp.clickMethodName.empty())
                    btnComp.onClickCallback(btnComp.scriptEntity, btnComp.clickMethodName);
            });


        return m_sceneGraph.createEntity(std::move(entity));
    }

    void Scene::setEditorCamera(IEditorCamera::Ptr editorCamera) {
        m_editorCamera = editorCamera;
        m_scene.getSystem<UISystem>() -> setCamera(m_editorCamera);
    }

    void Scene::registerUICallback(SceneEntity uiEntity) {
        uiEntity.addComponent<UIHitbox>().callbackIDs[UIHitbox::CallbackID::MouseDown] =
            m_scene.getSystem<UISystem>() -> addMousePressedCallback([](robot2D::ecs::Entity entity, std::uint64_t) {
                auto& btnComp = entity.getComponent<ButtonComponent>();
                if (btnComp.hasEntity()
                    && btnComp.onClickCallback && !btnComp.clickMethodName.empty())
                    btnComp.onClickCallback(btnComp.scriptEntity, btnComp.clickMethodName);
            });
    }

    SceneEntity Scene::duplicateRuntime(SceneEntity entity, robot2D::vec2f position) {

        auto dupEntity = m_scene.duplicateEntity(entity.getWrappedEntity());
        dupEntity.getComponent<TagComponent>().setTag("RuntimeClone");
        dupEntity.getComponent<IDComponent>().ID = UUID();
        dupEntity.getComponent<TransformComponent>().setPosition(position);
        auto sceneEntity = SceneEntity{dupEntity};
        m_physicsAdapter -> addRuntime(sceneEntity);

        if (entity.isChild()) {
            auto parent = entity.getParent();
            parent -> addChild(sceneEntity);
        } else
            m_runtimeSceneGraph.addEntity(sceneEntity);

        return sceneEntity;
    }

    void Scene::traverseGraph(TraverseFunction&& traverseFunction) {
        m_sceneGraph.traverseGraph(std::move(traverseFunction));
    }


}