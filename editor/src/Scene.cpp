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
        m_sceneGraph.addEntity(SceneEntity{ std::move(entity) });
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
        for(auto& item: m_setItems) {
            auto iter = std::get<0>(item);
            if(iter == sceneEntities.begin())
                sceneEntities.insert(iter, std::get<1>(item));
            bool isChained = std::get<2>(item);
            if (!isChained) {
                if(iter != sceneEntities.end())
                    sceneEntities.insert(std::next(iter), std::get<1>(item));
                else
                    sceneEntities.insert(iter, std::get<1>(item));
            }
            else {
                auto anchor = std::get<3>(item);
                auto prevFound = std::find_if(sceneEntities.begin(), sceneEntities.end(),
                                              [&anchor](auto item) {
                                                  return item == anchor;
                                              });
                sceneEntities.insert(std::next(prevFound), std::get<1>(item));
            }
        }

        m_setItems.clear();
        m_sceneGraph.updateSelf(m_scene);
        m_scene.update(dt);
    }

    void Scene::updateRuntime(float dt) {
        /// TODO(a.raag): moving to SceneGraph
        for(auto& entity: m_sceneGraph.getEntities()) {
            if(entity.hasComponent<ScriptComponent>())
                ScriptEngine::onUpdateEntity(entity, dt);
            if(entity.hasChildren()) {
                for(const auto& child: entity.getChildren()) {
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

    SceneEntity Scene::createEntity() {
        auto entity = m_scene.createEntity();
        return m_sceneGraph.createEntity(std::move(entity));
    }

    void Scene::addAssociatedEntity(SceneEntity&& entity) {
        m_sceneGraph.addEntity(std::move(entity));
    }


    SceneEntity Scene::createEmptyEntity() {
        return SceneEntity(std::move(m_scene.createEmptyEntity()));
    }

    void Scene::addEmptyEntity() {
        auto entity = m_scene.createEntity();
        entity.addComponent<IDComponent>(UUID());
        entity.addComponent<TagComponent>();

        auto& transform = entity.addComponent<TransformComponent>();
        transform.setPosition({100.F, 100.F});
        transform.setScale({ 20.F, 20.F });
        entity.addComponent<DrawableComponent>();

        m_sceneGraph.addEntity(SceneEntity{ std::move(entity) });
        m_hasChanges = true;
    }


    void Scene::removeEntity(SceneEntity entity) {
   /*     auto found = std::find_if(m_sceneEntities.begin(),
                                  m_sceneEntities.end(),
                                  [&entity](robot2D::ecs::Entity ent) {
            return ent.getIndex() == entity.getIndex();
        });
        if(found == m_sceneEntities.end())
            return;
        m_deletePendingBuffer.emplace_back(*found);
        m_hasChanges = true;*/
    }


    void Scene::onRuntimeStart(ScriptInteractor::Ptr scriptInteractor) {
        m_running = true;
        m_scene.cloneSelf(m_CloneScene);
        onPhysics2DRun();

        m_scene.getSystem<RenderSystem>() -> setScene(this);
        ScriptEngine::onRuntimeStart(scriptInteractor);

        /// TODO(a.raag): moving to SceneGraph
        for(auto& entity: m_sceneGraph.getEntities()) {
            if(entity.hasComponent<ScriptComponent>() && !entity.hasComponent<PrefabComponent>())
                ScriptEngine::onCreateEntity(entity);
            if(entity.hasChildren()) {
                for(const auto& child: entity.getChildren()) {
                    if(child.hasComponent<ScriptComponent>())
                        ScriptEngine::onCreateEntity(child);
                }
            }

            if(entity.hasComponent<ButtonComponent>()) {
                auto& hitbox = entity.getComponent<UIHitbox>();
                auto& ts = entity.getComponent<TransformComponent>();
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
    }

    void Scene::onPhysics2DRun() {
        m_physicsAdapter = getPhysics2DAdapter(PhysicsAdapterType::Box2D);
        m_physicsAdapter -> start(m_sceneGraph.getEntities());


        m_physicsAdapter -> registerCallback(PhysicsCallbackType::Enter,
                                             [](const Physics2DContact& contact) {
            ScriptEngine::onCollision2DBegin(contact);
        });

        m_physicsAdapter -> registerCallback(PhysicsCallbackType::Exit,
                                             [](const Physics2DContact& contact) {
            ScriptEngine::onCollision2DEnd(contact);
        });

        m_physicsAdapter -> registerCallback(PhysicsCallbackType::EnterTrigger,
                                             [](const Physics2DContact& contact) {
            ScriptEngine::onCollision2DBeginTrigger(contact);
        });

        m_physicsAdapter -> registerCallback(PhysicsCallbackType::ExitTrigger,
                                             [](const Physics2DContact& contact) {
            ScriptEngine::onCollision2DEndTrigger(contact);
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
        m_scene.getSystem<RenderSystem>() -> setRuntimeFlag(flag);
    }

    SceneEntity Scene::getEntity(UUID uuid) const{
        return m_sceneGraph.getEntity(uuid);
    }


    void Scene::setBefore(SceneEntity source, SceneEntity target) {
        if (m_sceneGraph.setBefore(source, target)) {
            m_scene.getSystem<RenderSystem>() -> setBefore(source.getWrappedEntity(),
                                                           target.getWrappedEntity());
            m_hasChanges = true;
            // return true;
        }

        // return false;
    }

    SceneEntity Scene::duplicateEntity(robot2D::vec2f mousePos, SceneEntity entity) {
        auto dupEntity = m_scene.duplicateEntity(entity.getWrappedEntity());
        std::string name = entity.getComponent<TagComponent>().getTag();
        name += "(Clone)";
        dupEntity.getComponent<IDComponent>().ID = UUID();
        dupEntity.getComponent<TagComponent>().setTag(name);

        auto* msg = m_messageBus.postMessage<EntityDuplication>(MessageID::EntityDuplicate);
        msg -> entityID = dupEntity.getComponent<IDComponent>().ID;

        /// add or create ???
        // m_sceneGraph.emplace_back(dupEntity);
        m_hasChanges = true;

        if(entity.hasChildren())
            duplicateEntityChild(SceneEntity{std::move(dupEntity)}, entity);

        dupEntity.getComponent<TransformComponent>().setPosition(mousePos);
        return SceneEntity(std::move(dupEntity));
    }

    void Scene::duplicateEntityChild(SceneEntity parent, SceneEntity dupEntity) {
        auto& parentTransform = dupEntity.getComponent<TransformComponent>();
        auto& dupParentTransform = parent.getComponent<TransformComponent>();
        dupParentTransform.clearChildren();



        //for(auto& child: parentTransform.getChildren()) {
        //    auto dupChild = m_scene.duplicateEntity(child);
        //    dupParentTransform.addChild(parent, dupChild);
        //    if(child.getComponent<TransformComponent>().hasChildren())
        //        duplicateEntityChild(dupChild, child);
        //}
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

        std::vector<RemoveEntityInfo> removeInfos{ removingEntities.size() };
        std::transform(removingEntities.begin(), removingEntities.end(), removeInfos.begin(),
                       [](SceneEntity entity) {
           return RemoveEntityInfo{entity};
        });
        using FoundIterator = std::list<SceneEntity>::const_iterator;


        const auto& sceneEntities = m_sceneGraph.getEntities();

        for(auto& item: removeInfos) {
            FoundIterator found = sceneEntities.begin();

            bool isParentDel = false;
            for(; found != sceneEntities.end(); ++found) {
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

            if(found == sceneEntities.end())
                continue;

            
            m_sceneGraph.removeEntity(item.entity);
            RestoreInfo restoreInfo; // first isChained child

            if(!restoreInformation.hasItems()) {
                if(found == sceneEntities.begin()) {
                    restoreInfo.first = true;
                    restoreInfo.entity = item.entity;
                }
                else {
                    restoreInfo.anchorEntity = *std::prev(found);
                    restoreInfo.entity = item.entity;
                }
            }
            else {
               auto lastInfo = restoreInformation.getLast();
               if(lastInfo.entity == *(std::prev(found))) {
                   restoreInfo.anchorEntity = lastInfo.entity;
                   restoreInfo.entity = *found;
                   restoreInfo.isChained = true;
               }
               else {
                   if(found == sceneEntities.begin()) {
                       restoreInfo.entity = *found;
                       restoreInfo.first = true;
                   }
                   else {
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


    void Scene::removeChildEntities(DeletedEntitiesRestoreInformation& information,
                                    std::vector<RemoveEntityInfo>& removingEntities,
                                    const SceneEntity& parent,
                                    bool isParentDel) {

        for(auto& item: removingEntities) {
            for(auto c: parent.getChildren()) {
                SceneEntity child{ std::move(c) };
                if(!child)
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
                    m_scene.removeEntity(child.getWrappedEntity());
                }

                if(isParentDel && !item.isDeleted) {
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

        for(auto& info: restoreInformation.getInfos()) {
            if(info.child) {
                if(info.anchorEntity)
                    info.anchorEntity.addChild(info.entity);
                else
                    childBuffer.emplace_back(info.anchorEntity, info.entity);

                m_scene.restoreEntity(info.entity.getWrappedEntity());
                info.entity.getComponent<UIComponent>().setName(&info.entity.getComponent<TagComponent>().getTag());
            }
            else {
                auto& sceneEntities = m_sceneGraph.getEntities();
                if(info.first) {
                    m_scene.restoreEntity(info.entity.getWrappedEntity());
                    info.entity.getComponent<UIComponent>().setName(&info.entity.getComponent<TagComponent>().getTag());
                    bool isChained = false;
                    m_setItems.emplace_back(sceneEntities.begin(), info.entity, isChained, SceneEntity{});
                }
                else {
                    if(!info.isChained) {
                        if(!m_scene.restoreEntity(info.entity.getWrappedEntity())) {
                            RB_EDITOR_ERROR("EditorScene: Can't Restore entity with index {0}",
                                            info.entity.getWrappedEntity().getIndex());
                        }
                        info.entity.getComponent<UIComponent>().setName(&info.entity.getComponent<TagComponent>().getTag());
                        auto anchorFound = std::find_if(sceneEntities.begin(),
                                                        sceneEntities.end(), [&info](auto item) {
                            return item == info.anchorEntity;
                        });
                        bool isChained = false;
                        m_setItems.emplace_back(anchorFound, info.entity, isChained, SceneEntity{});
                    }
                    else {
                        if(!m_scene.restoreEntity(info.entity.getWrappedEntity())) {
                            RB_EDITOR_ERROR("EditorScene: Can't Restore entity with index {0}",
                                            info.entity.getWrappedEntity().getIndex());
                        };
                        info.entity.getComponent<UIComponent>().setName(&info.entity.getComponent<TagComponent>().getTag());
                        bool isChained = true;
                        m_setItems.emplace_back(sceneEntities.end(), info.entity, isChained, info.anchorEntity);
                    }
                }

            }

        }

        for (auto& [parent, child]: childBuffer)
            parent.addChild(child);

    }

    SceneEntity Scene::addEmptyButton() {
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

       
        return m_sceneGraph.createEntity(std::move(entity));
    }

    void Scene::setEditorCamera(IEditorCamera::Ptr editorCamera) {
        m_editorCamera = editorCamera;
        m_scene.getSystem<UISystem>() -> setCamera(m_editorCamera);
    }

    void Scene::registerUICallback(SceneEntity uiEntity) {
        uiEntity.addComponent<UIHitbox>().callbackIDs[UIHitbox::CallbackID::MouseDown] =
                m_scene.getSystem<UISystem>() -> addMousePressedCallback([](robot2D::ecs::Entity entity, std::uint64_t){
                    auto& btnComp = entity.getComponent<ButtonComponent>();
                    if(btnComp.hasEntity()
                       && btnComp.onClickCallback && !btnComp.clickMethodName.empty())
                        btnComp.onClickCallback(btnComp.scriptEntity, btnComp.clickMethodName);
                });
    }

    SceneEntity Scene::duplicateRuntime(SceneEntity entity, robot2D::vec2f position) {

        auto dupEntity = m_scene.duplicateEntity(entity.getWrappedEntity());
        dupEntity.getComponent<TagComponent>().setTag("RuntimeClone");
        dupEntity.getComponent<IDComponent>().ID = UUID();
        dupEntity.getComponent<TransformComponent>().setPosition(position);
        
        /// TODO(a.raag): add to runtime ???
        /// m_sceneEntities.emplace_back(dupEntity);

        m_physicsAdapter -> addRuntime(entity.getWrappedEntity());
        return dupEntity;
    }
}

