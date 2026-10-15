/*********************************************************************
(c) Alex Raag 2026
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
#include <unordered_set>
#include <mutex>
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
#include <editor/Messages.hpp>
#include <editor/physics/Layers2D.hpp>

namespace editor {

    namespace {
        constexpr robot2D::vec2f defaultPosition = {100.f, 100.f};
        constexpr robot2D::vec2f defaultSize = {20.f, 20.f};
    }

    Scene::Scene(robot2D::MessageBus &messageBus) :
            m_scene(messageBus),
            m_runtimeScene{nullptr},
            m_messageBus{messageBus} {
        initScene();
    }

    void Scene::initScene() {
        m_scene.addSystem<RenderSystem>(m_messageBus);
        m_scene.addSystem<TextSystem>(m_messageBus);
        m_scene.addSystem<AnimatorSystem>(m_messageBus);
        m_scene.addSystem<AnimationSystem>(m_messageBus);
        m_scene.addSystem<UISystem>(m_messageBus);

        static std::once_flag physicsLayersInitialized;
        std::call_once(physicsLayersInitialized, InitPhysicsLayers);
    }

    void Scene::createMainCamera() {
        auto entity = createEntity("MainCamera");
        entity.addComponent<CameraComponent>().isPrimary = true;
        entity.addComponent<DrawableComponent>();
    }

    const std::list<SceneEntity>& Scene::getEntities() const {
        return getRootEntities();
    }

    const std::list<SceneEntity>& Scene::getRootEntities(SceneContext context) const {
        return getStorage(context).roots;
    }

    void Scene::handleEventsRuntime(const robot2D::Event& event) {
        if(m_running && m_runtimeScene)
            m_runtimeScene -> getSystem<UISystem>() -> handleEvents(event);
    }

    void Scene::update(float dt) {
        processRemovals(SceneContext::Editor);
        m_scene.update(dt);
    }

    void Scene::updateRuntime(float dt, IScriptInteractorFrom::Ptr scriptInteractor) {
        if(!m_running || !m_runtimeScene)
            return;
        processRemovals(SceneContext::Runtime);
        auto scriptingEngine = scriptInteractor -> getScriptingEngine();
        if(!scriptingEngine)
            return;

        for (const auto& entity: m_scriptRuntimeContainer)
            scriptingEngine -> onUpdateEntity(entity, dt);

        m_physicsAdapter -> update(dt);
        m_runtimeScene -> update(dt);
    }

    void Scene::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
        if(m_running) {
            target.draw(*m_runtimeScene);
            return;
        }
        target.draw(m_scene);
    }

    SceneEntity Scene::createEntity(const std::string& name) {
        auto entity = createEmptyEntity();
        entity.addComponent<IDComponent>();
        entity.addComponent<TagComponent>().setTag(name);
        auto& transform = entity.addComponent<TransformComponent>();
        transform.setPosition(defaultPosition);
        transform.setSize(defaultSize);
        attachEntity(entity);
        m_hasChanges = true;
        return entity;
    }

    SceneEntity Scene::createEmptyEntity() {
        return wrapEntity(m_scene.createEmptyEntity(), SceneContext::Editor);
    }

    bool Scene::attachEntity(SceneEntity& entity) {
        if(entity.m_scene != this || !entity || entity.m_context != SceneContext::Editor
           || !entity.hasComponent<IDComponent>())
            return false;
        auto existing = getEntity(entity.getUUID(), SceneContext::Editor);
        if(existing && existing.getWrappedEntity() != entity.getWrappedEntity())
            return false;
        if(!registerEntity(entity))
            return false;
        entity = getEntity(entity.getUUID(), SceneContext::Editor);
        addRoot(entity);
        if(!existing)
            m_scene.addEntity(entity.getWrappedEntity());
        m_hasChanges = true;
        return true;
    }

    SceneEntity Scene::addEmptyEntity() {
        auto entity = createEntity();
        entity.addComponent<DrawableComponent>();
        return entity;
    }

    void Scene::onRuntimeStart(IScriptInteractorFrom::Ptr scriptInteractor) {
        if(m_running)
            return;
        ++m_runtimeVersion;
        m_runtimeClonedArray.clear();
        m_runtimeEntities = EntityStorage{};
        m_runtimeScene = std::make_unique<robot2D::ecs::Scene>(m_messageBus);
        m_running = true;
        if(!m_scene.cloneSelf(*m_runtimeScene, m_runtimeClonedArray, true)) {
            throw std::runtime_error("m_scene.cloneSelf exception");
        }

        m_runtimeEntities.entities.clear();
        m_runtimeEntities.roots.clear();
        m_scriptRuntimeContainer.clear();
        for(const auto& entity: m_runtimeClonedArray) {
            if(!entity.hasComponent<IDComponent>()) {
                m_runtimeScene -> removeEntity(entity);
                continue;
            }
            auto source = getEntity(entity.getComponent<IDComponent>().ID, SceneContext::Editor);
            if(!source || source.getWrappedEntity().getIndex() != entity.getIndex()) {
                m_runtimeScene -> removeEntity(entity);
                continue;
            }
            auto wrappedEntity = entity;
            auto newSceneEntity = wrapEntity(std::move(wrappedEntity), SceneContext::Runtime);
            if(!registerEntity(newSceneEntity))
                continue;
            if(entity.hasComponent<ScriptComponent>())
                m_scriptRuntimeContainer.push_back(newSceneEntity);
        }

        for(auto root: m_editorEntities.roots)
            addRoot(getEntity(root.getUUID(), SceneContext::Runtime));

        onPhysics2DRun(scriptInteractor);

        m_runtimeScene -> getSystem<RenderSystem>() -> setScene(this);
        auto scriptingEngine = scriptInteractor -> getScriptingEngine();
        if(!scriptingEngine)
            return;

        scriptingEngine -> onRuntimeStart(scriptInteractor);



        for (const auto& entity: m_scriptRuntimeContainer)
            scriptingEngine -> onCreateEntity(entity);


        forEachEntity(std::move([](SceneEntity& sceneEntity) {
            if (sceneEntity.hasComponent<ButtonComponent>()) {
                auto& hitbox = sceneEntity.getComponent<UIHitbox>();
                auto& ts = sceneEntity.getComponent<TransformComponent>();
                // TODO(a.raag) ts.getGlobalBounds();
                hitbox.m_area = robot2D::FloatRect::create(ts.getPosition(),
                                                           ts.getSize() + ts.getPosition());
            }
        }), SceneContext::Runtime);
    }

    void Scene::onRuntimeStop(IScriptInteractorFrom::Ptr scriptInteractor) {
        if(!m_running)
            return;
        m_runtimeScene -> getSystem<RenderSystem>() -> setScene(nullptr);
        onPhysics2DStop();
        auto scriptingEngine = scriptInteractor ? scriptInteractor -> getScriptingEngine() : nullptr;
        if(scriptingEngine)
            scriptingEngine -> onRuntimeStop();
        m_running = false;
        ++m_runtimeVersion;
        m_runtimeEntities = EntityStorage{};
        m_runtimeClonedArray.clear();
        m_scriptRuntimeContainer.clear();
        m_listPhysics.clear();
        m_runtimeScene.reset();
    }

    void Scene::onPhysics2DRun(IScriptInteractorFrom::Ptr scriptInteractor) {
        m_physicsAdapter = getPhysics2DAdapter(PhysicsAdapterType::Box2D);
        m_listPhysics.clear();

        for(auto entity: m_runtimeEntities.roots)
            m_listPhysics.emplace_back(entity);

        m_physicsAdapter -> start(m_listPhysics);
        auto scriptingEngine = scriptInteractor -> getScriptingEngine();
        if(!scriptingEngine)
            return;

        m_physicsAdapter -> registerCallback(
                [scriptingEngine](const PhysicsContact2D& contact, UUID self, UUID other) {
                scriptingEngine -> onPhysicsCallback(contact, self, other);
        });
    }

    void Scene::onPhysics2DStop() {
        m_physicsAdapter -> stop();
        m_physicsAdapter.reset();
    }


    void Scene::setRuntimeCamera(bool flag) {
        if(!m_running)
            m_scene.getSystem<RenderSystem>() -> setRuntimeFlag(flag);
        else
            m_runtimeScene -> getSystem<RenderSystem>() -> setRuntimeFlag(flag);
    }

    void Scene::setRuntimeWindowSize(const robot2D::vec2u& size) {
        if(!m_running)
            m_scene.getSystem<RenderSystem>() -> setRuntimeWindowSize(size);
        else
            m_runtimeScene -> getSystem<RenderSystem>() -> setRuntimeWindowSize(size);
    }

    SceneEntity Scene::getEntity(UUID uuid) const {
        return getEntity(uuid, m_running ? SceneContext::Runtime : SceneContext::Editor);
    }

    SceneEntity Scene::getEntity(UUID uuid, SceneContext context) const {
        const auto& entities = getStorage(context).entities;
        auto found = entities.find(uuid);
        if(found != entities.end() && found -> second)
            return found -> second;
        return {};
    }

    SceneEntity Scene::getEntity(const std::string& name) const {
        const auto& entities = getStorage(m_running ? SceneContext::Runtime : SceneContext::Editor).entities;
        for(const auto& [uuid, entity]: entities) {
            if(entity && entity.hasComponent<TagComponent>() && entity.getName() == name)
                return entity;
        }
        return {};
    }

    bool Scene::setBefore(SceneEntity source, SceneEntity target) {
        if(!reorderEntity(source, target))
            return false;
        auto* ecs = source.m_context == SceneContext::Editor ? &m_scene : m_runtimeScene.get();
        if(ecs && source.hasComponent<DrawableComponent>() && target.hasComponent<DrawableComponent>())
            ecs -> getSystem<RenderSystem>() -> setBefore(source.getWrappedEntity(), target.getWrappedEntity());
        if(source.m_context == SceneContext::Editor)
            m_hasChanges = true;
        return true;
    }

    SceneEntity Scene::duplicateEntity(robot2D::vec2f mousePos, SceneEntity entity) {
        if(!ownsEntity(entity) || entity.m_context != SceneContext::Editor)
            return {};
        // TODO(a.raag): if clone not add "(Clone)"

        auto dupEntity = m_scene.duplicateEntity(entity.getWrappedEntity());
        std::string name = entity.getComponent<TagComponent>().getTag();
        name += "(Clone)";
        dupEntity.getComponent<IDComponent>().ID = UUID();
        dupEntity.getComponent<TagComponent>().setTag(name);

        auto* msg =
                m_messageBus.postMessage<EntityDuplication>(MessageID::EntityDuplicate);
        msg -> entityID = dupEntity.getComponent<IDComponent>().ID;
        m_hasChanges = true;

        auto duplicateSceneEntity = wrapEntity(std::move(dupEntity), SceneContext::Editor);
        if(duplicateSceneEntity.hasComponent<HierarchyComponent>())
            duplicateSceneEntity.getComponent<HierarchyComponent>() = HierarchyComponent{};
        addRoot(duplicateSceneEntity);
        if(entity.isChild()) {
            auto parent = entity.getParent();
            parent.addChild(duplicateSceneEntity);
        }

        if (entity.hasChildren())
            duplicateEntityChild(duplicateSceneEntity, entity);

        return duplicateSceneEntity;
    }

    void Scene::duplicateEntityChild(SceneEntity parent, SceneEntity source) {
        for(auto child: source.getChildren()) {
            auto duplicated = m_scene.duplicateEntity(child.getWrappedEntity());
            duplicated.getComponent<IDComponent>().ID = UUID();
            auto duplicateChild = wrapEntity(std::move(duplicated), SceneContext::Editor);
            if(duplicateChild.hasComponent<HierarchyComponent>())
                duplicateChild.getComponent<HierarchyComponent>() = HierarchyComponent{};
            addRoot(duplicateChild);
            reparent(duplicateChild, parent);
            duplicateEntityChild(duplicateChild, child);
        }
    }

    SceneEntity Scene::duplicateEmptyEntity(SceneEntity entity) {
        if(entity.m_scene != this || !entity || entity.m_context != SceneContext::Editor
           || !entity.hasComponent<IDComponent>())
            return {};
        auto dupEntity = m_scene.duplicateEntity(entity.getWrappedEntity());
        dupEntity.getComponent<IDComponent>().ID = UUID();

        auto duplicate = wrapEntity(std::move(dupEntity), SceneContext::Editor);
        if(duplicate.hasComponent<HierarchyComponent>())
            duplicate.getComponent<HierarchyComponent>() = HierarchyComponent{};
        addRoot(duplicate);
        return duplicate;
    }

    void Scene::removeDuplicate(SceneEntity entity) {
        m_hasChanges = true;
        queueRemoval(entity);
    }


    DeletedEntitiesRestoreInformation Scene::removeEntities(std::vector<SceneEntity>& entities) {
        DeletedEntitiesRestoreInformation information;
        if(m_running)
            return information;
        std::unordered_set<UUID> selected;
        for(auto entity: entities) {
            if(ownsEntity(entity) && entity.m_context == SceneContext::Editor)
                selected.insert(entity.getUUID());
        }
        std::unordered_set<UUID> visited;
        std::function<void(SceneEntity)> collect = [&](SceneEntity entity) {
            if(!entity || !visited.insert(entity.getUUID()).second)
                return;
            DeletedEntitiesRestoreInformation::RestoreInfo info;
            info.entity = entity;
            auto parent = entity.getParent();
            info.child = static_cast<bool>(parent);
            info.anchorEntity = parent;
            auto siblings = parent ? parent.getChildren()
                                   : std::vector<SceneEntity>(m_editorEntities.roots.begin(),
                                                              m_editorEntities.roots.end());
            auto found = std::find(siblings.begin(), siblings.end(), entity);
            if(found != siblings.end()) {
                info.first = found == siblings.begin();
                if(std::next(found) != siblings.end())
                    info.nextSiblingID = std::next(found) -> getUUID();
                if(!parent && found != siblings.begin())
                    info.anchorEntity = *std::prev(found);
            }
            information.push(std::move(info));
            for(auto child: entity.getChildren())
                collect(child);
        };
        for(auto entity: entities) {
            if(!ownsEntity(entity) || entity.m_context != SceneContext::Editor)
                continue;
            bool parentSelected = false;
            for(auto parent = entity.getParent(); parent; parent = parent.getParent()) {
                if(selected.find(parent.getUUID()) != selected.end()) {
                    parentSelected = true;
                    break;
                }
            }
            if(parentSelected)
                continue;
            collect(entity);
            queueRemoval(entity);
        }
        if(information.hasItems())
            m_hasChanges = true;
        return information;
    }

    void Scene::restoreEntities(DeletedEntitiesRestoreInformation& information) {
        if(m_running)
            return;
        for(const auto& info: information.getInfos()) {
            if(info.entity.m_scene != this || info.entity.m_context != SceneContext::Editor)
                return;
        }
        for(auto& info: information.getInfos()) {
            cancelPendingRemoval(info.entity);
            if(!info.entity && !m_scene.restoreEntity(info.entity.getWrappedEntity())) {
                RB_EDITOR_ERROR("EditorScene: Can't restore entity with index {0}",
                                info.entity.getWrappedEntity().getIndex());
                return;
            }
            if(info.entity.hasComponent<HierarchyComponent>())
                info.entity.getComponent<HierarchyComponent>() = HierarchyComponent{};
            registerEntity(info.entity);
        }
        for(auto& info: information.getInfos()) {
            if(info.child)
                reparent(info.entity, info.anchorEntity);
            else
                addRoot(info.entity);
        }
        for(auto iter = information.getInfos().rbegin(); iter != information.getInfos().rend(); ++iter) {
            if(iter -> nextSiblingID) {
                auto sibling = getEntity(*iter -> nextSiblingID);
                if(sibling)
                    reorderEntity(iter -> entity, sibling);
            }
        }
        m_hasChanges = true;
    }

    SceneEntity Scene::addEmptyButton() {
        auto entity = createEntity();
        entity.addComponent<ButtonComponent>();
        entity.addComponent<UIHitbox>().callbackIDs[UIHitbox::CallbackID::MouseDown] =
            m_scene.getSystem<UISystem>() -> addMousePressedCallback([](robot2D::ecs::Entity entity, std::uint64_t) {
                auto& button = entity.getComponent<ButtonComponent>();
                if(button.hasEntity() && button.onClickCallback && !button.clickMethodName.empty())
                    button.onClickCallback(button.scriptEntity, button.clickMethodName);
            });
        return entity;
    }

    void Scene::setEditorCamera(IEditorCamera::Ptr editorCamera) {
        m_editorCamera = editorCamera;
        m_scene.getSystem<UISystem>() -> setCamera(m_editorCamera);
    }

    void Scene::registerUICallback(SceneEntity uiEntity) {
        if(!ownsEntity(uiEntity) || uiEntity.m_context != SceneContext::Editor
           || !uiEntity.hasComponent<ButtonComponent>())
            return;
        uiEntity.addComponent<UIHitbox>().callbackIDs[UIHitbox::CallbackID::MouseDown] =
            m_scene.getSystem<UISystem>() -> addMousePressedCallback([](robot2D::ecs::Entity entity, std::uint64_t) {
                auto& btnComp = entity.getComponent<ButtonComponent>();
                if (btnComp.hasEntity()
                    && btnComp.onClickCallback && !btnComp.clickMethodName.empty())
                    btnComp.onClickCallback(btnComp.scriptEntity, btnComp.clickMethodName);
            });
    }

    SceneEntity Scene::duplicateRuntime(SceneEntity entity, robot2D::vec2f position) {
        if(!m_running || !m_runtimeScene || !ownsEntity(entity) || entity.m_context != SceneContext::Runtime)
            return {};

        auto dupEntity = m_runtimeScene -> duplicateEntity(entity.getWrappedEntity());
        dupEntity.getComponent<TagComponent>().setTag("RuntimeClone");
        dupEntity.getComponent<IDComponent>().ID = UUID();
        auto sceneEntity = wrapEntity(std::move(dupEntity), SceneContext::Runtime);
        if(sceneEntity.hasComponent<HierarchyComponent>())
            sceneEntity.getComponent<HierarchyComponent>() = HierarchyComponent{};
        addRoot(sceneEntity);
        sceneEntity.getComponent<TransformComponent>().setPosition(position);
        m_physicsAdapter -> addRuntime(sceneEntity);

        if (entity.isChild()) {
            auto parent = entity.getParent();
            parent.addChild(sceneEntity);
        }

        return sceneEntity;
    }



    Scene::EntityStorage& Scene::getStorage(SceneContext context) {
        return context == SceneContext::Editor ? m_editorEntities : m_runtimeEntities;
    }

    const Scene::EntityStorage& Scene::getStorage(SceneContext context) const {
        return context == SceneContext::Editor ? m_editorEntities : m_runtimeEntities;
    }

    bool Scene::isContextValid(SceneContext context, std::uint64_t version) const {
        return context == SceneContext::Editor ? version == 0
                                              : m_running && version == m_runtimeVersion;
    }

    SceneEntity Scene::wrapEntity(robot2D::ecs::Entity entity, SceneContext context) {
        SceneEntity result{entity};
        result.m_scene = this;
        result.m_context = context;
        result.m_contextVersion = context == SceneContext::Editor ? 0 : m_runtimeVersion;
        if(result.hasComponent<IDComponent>())
            result.m_uuid = entity.getComponent<IDComponent>().ID;
        return result;
    }

    bool Scene::registerEntity(SceneEntity entity) {
        if(entity.m_scene != this || !entity || !entity.hasComponent<IDComponent>())
            return false;
        auto& storage = getStorage(entity.m_context);
        auto uuid = entity.getUUID();
        auto found = storage.entities.find(uuid);
        if(found != storage.entities.end() && found -> second.getWrappedEntity() != entity.getWrappedEntity())
            return false;
        entity.m_uuid = uuid;
        if(!entity.hasComponent<HierarchyComponent>())
            entity.addComponent<HierarchyComponent>();
        if(entity.hasComponent<TransformComponent>()) {
            auto& transform = entity.getComponent<TransformComponent>();
            transform.m_scene = this;
            transform.m_entityID = uuid;
            transform.m_context = entity.m_context;
            transform.m_contextVersion = entity.m_contextVersion;
        }
        storage.entities[uuid] = entity;
        return true;
    }

    bool Scene::ownsEntity(SceneEntity entity) const {
        if(entity.m_scene != this || !entity || !entity.hasComponent<IDComponent>())
            return false;
        return getEntity(entity.getUUID(), entity.m_context) == entity;
    }

    void Scene::addRoot(SceneEntity entity) {
        if(!registerEntity(entity))
            return;
        auto self = getEntity(entity.getUUID(), entity.m_context);
        auto& roots = getStorage(entity.m_context).roots;
        if(!self.isChild() && std::find(roots.begin(), roots.end(), self) == roots.end())
            roots.emplace_back(self);
    }

    std::vector<SceneEntity> Scene::getChildren(SceneEntity entity) const {
        std::vector<SceneEntity> children;
        if(!ownsEntity(entity) || !entity.hasComponent<HierarchyComponent>())
            return children;
        for(auto uuid: entity.getComponent<HierarchyComponent>().getChildrenIDs()) {
            auto child = getEntity(uuid, entity.m_context);
            if(child)
                children.emplace_back(child);
        }
        return children;
    }

    SceneEntity Scene::getParent(SceneEntity entity) const {
        if(!ownsEntity(entity) || !entity.hasComponent<HierarchyComponent>())
            return {};
        auto parent = entity.getComponent<HierarchyComponent>().getParentID();
        return parent ? getEntity(*parent, entity.m_context) : SceneEntity{};
    }

    bool Scene::reparent(SceneEntity entity, SceneEntity parent) {
        if(!ownsEntity(entity) || ((parent.m_scene || parent.getWrappedEntity()) && !ownsEntity(parent)))
            return false;
        auto context = entity.m_context;
        if(context != (m_running ? SceneContext::Runtime : SceneContext::Editor)
           || (parent && parent.m_context != context))
            return false;
        if(parent) {
            std::unordered_set<UUID> visited;
            for(auto ancestor = parent; ancestor; ancestor = ancestor.getParent()) {
                if(ancestor == entity || !visited.insert(ancestor.getUUID()).second)
                    return false;
            }
        }
        auto& storage = getStorage(context);
        auto oldParent = entity.getParent();
        auto& hierarchy = entity.getComponent<HierarchyComponent>();
        if(oldParent == parent && (parent || !hierarchy.m_parentID)) {
            if(!parent)
                addRoot(entity);
            return true;
        }
        if(oldParent)
            oldParent.getComponent<HierarchyComponent>().m_childrenIDs.remove(entity.getUUID());
        storage.roots.remove(entity);
        hierarchy.m_parentID.reset();
        if(parent) {
            hierarchy.m_parentID = parent.getUUID();
            auto& children = parent.getComponent<HierarchyComponent>().m_childrenIDs;
            if(std::find(children.begin(), children.end(), entity.getUUID()) == children.end())
                children.emplace_back(entity.getUUID());
        }
        else
            storage.roots.emplace_back(getEntity(entity.getUUID(), context));
        if(context == SceneContext::Editor)
            m_hasChanges = true;
        return true;
    }

    bool Scene::reorderEntity(SceneEntity source, SceneEntity target) {
        if(!ownsEntity(source) || !ownsEntity(target) || source == target
           || source.m_context != target.m_context || source.getParent() != target.getParent())
            return false;
        auto parent = source.getParent();
        if(parent) {
            auto& children = parent.getComponent<HierarchyComponent>().m_childrenIDs;
            auto from = std::find(children.begin(), children.end(), source.getUUID());
            auto to = std::find(children.begin(), children.end(), target.getUUID());
            if(from == children.end() || to == children.end())
                return false;
            children.splice(to, children, from);
        }
        else {
            auto& roots = getStorage(source.m_context).roots;
            auto from = std::find(roots.begin(), roots.end(), source);
            auto to = std::find(roots.begin(), roots.end(), target);
            if(from == roots.end() || to == roots.end())
                return false;
            roots.splice(to, roots, from);
        }
        return true;
    }

    void Scene::moveChildren(UUID parentID, const robot2D::vec2f& offset,
                              SceneContext context, std::uint64_t version) {
        if(!isContextValid(context, version))
            return;
        auto parent = getEntity(parentID, context);
        for(auto child: getChildren(parent)) {
            if(child.hasComponent<TransformComponent>()) {
                auto& transform = child.getComponent<TransformComponent>();
                transform.setPosition(transform.getPosition() + offset);
            }
        }
    }

    void Scene::queueRemoval(SceneEntity entity) {
        if(ownsEntity(entity))
            getStorage(entity.m_context).pendingRemovals.emplace_back(entity);
    }

    void Scene::cancelPendingRemoval(SceneEntity entity) {
        auto& pending = getStorage(entity.m_context).pendingRemovals;
        pending.erase(std::remove(pending.begin(), pending.end(), entity), pending.end());
    }

    void Scene::processRemovals(SceneContext context) {
        auto& storage = getStorage(context);
        auto pending = std::move(storage.pendingRemovals);
        storage.pendingRemovals.clear();
        auto* ecs = context == SceneContext::Editor ? &m_scene : m_runtimeScene.get();
        if(!ecs)
            return;
        std::function<void(SceneEntity)> removeSubtree = [&](SceneEntity entity) {
            if(!ownsEntity(entity))
                return;
            for(auto child: getChildren(entity))
                removeSubtree(child);
            auto uuid = entity.getUUID();
            auto parent = getParent(entity);
            if(parent)
                parent.getComponent<HierarchyComponent>().m_childrenIDs.remove(uuid);
            storage.roots.remove(entity);
            storage.entities.erase(uuid);
            ecs -> removeEntity(entity.getWrappedEntity());
        };
        for(auto entity: pending)
            removeSubtree(entity);
    }

    void Scene::forEachEntity(TraverseFunction&& traverseFunction, SceneContext context) {
        auto roots = getRootEntities(context);
        for(auto entity: roots) {
            if(!entity)
                continue;
            traverseFunction(entity);
            forEachChild(traverseFunction, entity);
        }
    }

    void Scene::forEachChild(TraverseFunction& traverseFunction, SceneEntity parent) {
        for(auto entity: getChildren(parent)) {
            if(!entity)
                continue;
            traverseFunction(entity);
            forEachChild(traverseFunction, entity);
        }
    }

}
