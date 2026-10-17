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

#include <editor/EditorLogic.hpp>
#include <editor/AnimationPlayback.hpp>
#include <editor/PrefabManager.hpp>
#include <editor/Editor.hpp>
#include <editor/FileApi.hpp>
#include <editor/Messages.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/LocalResourceManager.hpp>
#include <editor/Components.hpp>

#include <editor/components/ButtonComponent.hpp>
#include <editor/scripting/ScriptingEngine.hpp>
#include <editor/scripting/ScriptProject.hpp>


#include <editor/commands/DuplicateCommand.hpp>
#include <editor/commands/DeleteEntitiesCommand.hpp>
#include <editor/commands/PasteCommand.hpp>
#include <editor/async/ExportTask.hpp>
#include <utility>


namespace {
    const std::string scenePath = "assets/scenes";
    const robot2D::FloatRect initRectangle = {{-10000, -10000}, {20000, 20000}};


}

namespace {
    /// (a.raag): Placeholder code for future
    class MemManager {
    public:
        static MemManager* getManager() {
            static MemManager manager;
            return &manager;
        }
        editor::Buffer allocateMemory(std::size_t memSize);

    };

    template<typename T, typename = std::enable_if_t<std::is_pod_v<T> > >
    class Array {
    public:
        Array(const editor::Buffer& buffer);

        void push_back(const T& value);
    private:
        T* m_arr;
        std::size_t size;
    };

}


namespace editor {
    namespace {
        class ScriptProjectTask final : public ITask {
        public:
            ScriptProjectTask(ITaskFunction::Ptr callback, Project::Ptr project, std::filesystem::path sources,
                ScriptProjectTools tools) : ITask(std::move(callback)), m_project(std::move(project)),
                m_sources(std::move(sources)), m_tools(std::move(tools)) {
            }
            void execute() override {
                result = m_sources.empty() ? buildProjectScripts(m_project -> getPath(), m_project -> getName(),
                        m_tools)
                    : generateProjectScripts(m_project -> getPath(), m_project -> getName(), m_sources, m_tools);
                if(!result.success) fail(result.diagnostic);
            }
            ScriptBuildResult result;
        private:
            Project::Ptr m_project;
            std::filesystem::path m_sources;
            ScriptProjectTools m_tools;
        };
    }
    EditorLogic::EditorLogic(robot2D::MessageBus& messageBus,
        MessageDispatcher& messageDispatcher,
        EditorPresenter& presenter,
        EditorRouter& router) :
        m_messageBus{messageBus},
        m_messageDispatcher{messageDispatcher},
        m_presenter{presenter},
        m_router{router},
        m_sceneManager{messageBus},
        m_activeScene{nullptr} {

        m_messageDispatcher.onMessage<MenuProjectMessage>(MessageID::SaveScene, BIND_CLASS_FN(saveScene));
        m_messageDispatcher.onMessage<MenuProjectMessage>(MessageID::SaveAll,
            [this](const MenuProjectMessage&) { saveAll(); });
        m_messageDispatcher.onMessage<OpenSceneMessage>(MessageID::OpenScene, BIND_CLASS_FN(openScene));
        m_messageDispatcher.onMessage<CreateSceneMessage>(MessageID::CreateScene, BIND_CLASS_FN(createScene));
        m_messageDispatcher.onMessage<PrefabLoadMessage>(MessageID::PrefabLoad, BIND_CLASS_FN(addPrefabEntity));
        m_messageDispatcher.onMessage<PrefabAssetModificatedMessage>(MessageID::PrefabAssetModificated,
            BIND_CLASS_FN(prefabModificated));

        m_messageDispatcher.onMessage<AnimationPlayMessage>(MessageID::AnimationPlay,
            [](const AnimationPlayMessage& message) {
            auto entity = message.entity;
            if(!entity || !entity.hasComponent<AnimationComponent>())
                return;
            auto* animation = entity.getComponent<AnimationComponent>().getAnimation();
            if(!animation)
                return;
            if(message.mode == AnimationPlayMessage::Mode::Play)
                playAnimation(entity, animation -> name);
            else
                stopAnimation(entity);
        });


        m_messageDispatcher.onMessage<GenerateProjectMessage>(MessageID::GenerateProject,
            BIND_CLASS_FN(generateProject));
        m_messageDispatcher.onMessage<BuildScriptsMessage>(MessageID::BuildScripts,
            [this](const BuildScriptsMessage&) { buildScripts(false); });

        m_quadTree.resizeRectangle(initRectangle);
    }


    void EditorLogic::handleEventsRuntime(const robot2D::Event& event) {
        // Focus/mode cancellation can arrive before async scene loading completes.
        if(m_activeScene)
            m_activeScene -> handleEventsRuntime(event);
    }

    void EditorLogic::update(float dt) {
        auto state = m_presenter.getState();
        switch(state) {
        case EditorState::Load:
            break;
        case EditorState::Edit: {
            for(auto& entity: getSelectedEntities()) {
                if(!entity)
                    continue;
                auto& tx = entity.getComponent<TransformComponent>();
                if(tx.m_hasModification) {
                    if(entity.hasComponent<QuadTreeComponent>()) {
                        auto iterator = entity.getComponent<QuadTreeComponent>().iterator;
                        m_quadTree.relocate(iterator, entity.calculateRect());
                    }
                    tx.m_hasModification = false;
                }
            }
            m_activeScene -> update(dt);
            break;
        }

        case EditorState::Run: {
            auto ptr = m_scriptInteractor.lock();
            m_activeScene -> updateRuntime(dt, ptr);
            break;
        }
        default:
            break;
        }
    }

    void EditorLogic::setup(IScriptInteractorFrom::WeakPtr scriptInteractor) {
        m_scriptInteractor = std::move(scriptInteractor);
        PopupManager::getManager() -> addObserver(this);
    }

    void EditorLogic::destroy() {
        RB_EDITOR_INFO("[EditorLogic] Waiting for task queue to stop");
        TaskQueue::GetQueue() -> stop();
        RB_EDITOR_INFO("[EditorLogic] Task queue stopped");
        if(m_activeScene && m_activeScene -> isRunning())
            m_activeScene -> onRuntimeStop(m_scriptInteractor.lock());
    }

    void EditorLogic::createProject(Project::Ptr project) {
        m_currentProject = project;
        if(!m_sceneManager.add(std::move(project), m_scriptInteractor)) {
            RB_EDITOR_ERROR("Can't Create Scene. Reason: {0}",
                errorToString(m_sceneManager.getError()));
            return;
        }
        m_selectionService.clear();
        m_commandStack.clear();
        m_copyEntities.clear();
        m_quadTree.clear();
        m_mainCameraEntity = {};
        m_activeScene = m_sceneManager.getActiveScene();

        m_activeScene -> createMainCamera();
        m_mainCameraEntity = m_activeScene -> getEntities().back();
        if(!saveScene()) {
            RB_EDITOR_ERROR("EditorLogic: can't save start version of scene.");
            return;
        }

        m_presenter.setMainCameraEntity(m_mainCameraEntity);
        m_router.openScene(m_activeScene, m_currentProject -> getPath());
    }

    void EditorLogic::loadProject(Project::Ptr project) {
        m_pendingProject = project;
        m_presenter.switchState(EditorState::Load);

        auto path = project -> getPath();
        auto appendPath = combinePath(scenePath, project -> getStartScene());
        auto scenePath = combinePath(path, appendPath);

        m_presenter.prepareView();
        m_sceneManager.loadSceneAsync(project, scenePath,
            BIND_CLASS_FN(loadSceneCallback), m_scriptInteractor);
    }

    /// TODO(a.raag): if scene don't have entities create main camera ???
    void EditorLogic::openScene(const OpenSceneMessage& message) {
        auto path = m_currentProject -> getPath();
        auto scenePath = combinePath(path, message.path);
        auto currentScenePath = combinePath(path, m_activeScene -> getPath());
        if(std::filesystem::path(scenePath) == std::filesystem::path(currentScenePath))
            return;

        auto taskQueue = TaskQueue::GetQueue();
        if(m_activeScene -> hasChanges() || m_presenter.hasAssetChanges() || taskQueue -> hasPendingTasks()) {
            m_popupConfiguration.title = "Close Scene";
            m_popupConfiguration.error.clear();
            m_popupConfiguration.onYes = [this, scenePath]() {
                if(!saveAll()) {
                    m_popupConfiguration.error =
                        "Could not save changes. Check the file path and write access, then retry.";
                    RB_EDITOR_ERROR("EditorLogic: can't save changes");
                    return false;
                }
                m_presenter.switchState(EditorState::Load);
                m_sceneManager.loadSceneAsync(m_currentProject, scenePath,
                    BIND_CLASS_FN(loadSceneCallback), m_scriptInteractor);
                return true;
            };

            m_popupConfiguration.onNo = [this, scenePath]() {
                m_presenter.discardAssetChanges();
                m_presenter.switchState(EditorState::Load);
                m_sceneManager.loadSceneAsync(m_currentProject, scenePath,
                    BIND_CLASS_FN(loadSceneCallback), m_scriptInteractor);
            };
            m_presenter.showPopup(&m_popupConfiguration);
        }
        else {
            m_presenter.switchState(EditorState::Load);
            m_sceneManager.loadSceneAsync(m_currentProject, scenePath,
                BIND_CLASS_FN(loadSceneCallback), m_scriptInteractor);
        }
    }

    void EditorLogic::createScene(const CreateSceneMessage& message) {
        if(!m_sceneManager.add(std::move(m_currentProject), message.path, m_scriptInteractor)) {
            RB_EDITOR_ERROR("Can't Create Scene. Reason: {0}",
                errorToString(m_sceneManager.getError()));
            return;
        }
    }

    void EditorLogic::saveScene(const MenuProjectMessage& message) {
        if(!m_sceneManager.save(std::move(m_activeScene), m_scriptInteractor, message.path))
            RB_EDITOR_ERROR("EditorLogic: can't save scene to {0}", message.path);
    }

    bool EditorLogic::saveScene() {
        const bool saved = m_sceneManager.save(std::move(m_activeScene), m_scriptInteractor);
        if(!saved)
            RB_EDITOR_ERROR("EditorLogic: can't save scene");
        return saved;
    }

    bool EditorLogic::saveAll() {
        return m_presenter.saveAssetChanges() && (!m_activeScene || saveScene());
    }

    void EditorLogic::loadSceneCallback(Scene::Ptr loadedScene) {
        if(!loadedScene) {
            m_pendingProject.reset();
            m_presenter.switchState(m_activeScene ? EditorState::Edit : EditorState::LoadFailed);
            RB_EDITOR_ERROR("[EditorLogic] Scene load failed; previous scene preserved={}",
                static_cast<bool>(m_activeScene));
            return;
        }
        if(m_pendingProject)
            m_currentProject = std::move(m_pendingProject);
        RB_EDITOR_INFO("[EditorLogic] Applying scene to editor; path='{}'", loadedScene -> getPath());
        m_selectionService.clear();
        m_commandStack.clear();
        m_copyEntities.clear();
        m_quadTree.clear();
        m_mainCameraEntity = {};
        m_activeScene = loadedScene;
        m_presenter.switchState(EditorState::Edit);
        m_presenter.setMainCameraEntity({});

        for(auto& entity: m_activeScene -> getEntities()) {
            processEntity(entity);

            if(entity.hasComponent<CameraComponent>()) {
                auto& cameraComponent = entity.getComponent<CameraComponent>();
                if(cameraComponent.isPrimary) {
                    m_mainCameraEntity = entity;
                    m_presenter.setMainCameraEntity(m_mainCameraEntity);
                }
            }
        }

        auto filename = std::filesystem::path{m_activeScene -> getPath()}.filename().string();
        m_currentProject -> setStartScene(filename);


        // TODO(a.raag): update project's start scene
        // m_currentProject -> save();
        m_router.openScene(m_activeScene, m_currentProject -> getPath());
        RB_EDITOR_INFO("[EditorLogic] Scene applied; path='{}'; entities={}",
            m_activeScene -> getPath(), m_activeScene -> getEntities().size());
    }


    void EditorLogic::copyToBuffer() {
        m_copyEntities.clear();
        m_copyEntities = getSelectedEntities();
    }

    void EditorLogic::pasterFromBuffer() {
        std::vector<SceneEntity> copiedEntities{};
        int counter = 0;

        struct PasteInfo {
            bool isParentPasted{false};
            SceneEntity entity;
        };

        /// child, child -> ???
        // m_mousePos;
        for(auto copy: m_copyEntities) {
            auto& transform = copy.getComponent<TransformComponent>();
            auto copiedEntity = m_activeScene -> duplicateEntity(transform.getPosition(), copy);
            auto rect = copiedEntity.calculateRect();
            auto treeIterator = m_quadTree.insert(copiedEntity, rect);
            if(!copiedEntity.hasComponent<QuadTreeComponent>())
                copiedEntity.addComponent<QuadTreeComponent>().iterator = treeIterator;
            else
                copiedEntity.getComponent<QuadTreeComponent>().iterator = treeIterator;
            copiedEntities.emplace_back(copiedEntity);
        }

        auto command = m_commandStack.addCommand<PasteCommand>(m_messageBus,
                std::move(copiedEntities), this);
        if(!command) {
            RB_EDITOR_ERROR("[EditorLogic]: [!] Can't Create PasteCommand");
        }
    }


    void EditorLogic::pasteChild(SceneEntity parent) {
    }


    void EditorLogic::switchRuntimeState(const ToolbarMessage::Type& type) {
        if(!m_activeScene) return;
        if(type == ToolbarMessage::Type::Start) {
            if(!m_activeScene -> isRunning()) buildScripts(true);
        }
        else if(type == ToolbarMessage::Type::Stop) {
            m_startAfterScriptBuild = false;
            m_activeScene -> onRuntimeStop(m_scriptInteractor.lock());
            m_presenter.switchState(EditorState::Edit);
        }
    }

    void EditorLogic::processEntity(SceneEntity entity) {
        if(entity.hasComponent<TransformComponent>()) {
            auto iterator = m_quadTree.insert(entity, entity.calculateRect());
            entity.addComponent<QuadTreeComponent>().iterator = iterator;
        }

        auto* manager = ResourceManager::getManager();
        auto* localManager = LocalResourceManager::getManager();

        if(entity.hasComponent<ButtonComponent>()) {
            auto& btnComp = entity.getComponent<ButtonComponent>();
            if(!btnComp.onClickCallback) {
                m_activeScene -> registerUICallback(entity);

                btnComp.onClickCallback = [this](UUID scriptUUID, const std::string& methodName) {
                    auto ptr = m_scriptInteractor.lock();
                    if(ptr)
                        ptr -> callScriptMethod(scriptUUID, methodName);
                };
            }
        }

        if(entity.hasComponent<DrawableComponent>()) {
            auto& drawable = entity.getComponent<DrawableComponent>();
            auto id = drawable.getTexturePath().empty() ? std::string{} : resourceFileKey(
                combinePath(m_activeScene -> getAssociatedProjectPath(), drawable.getTexturePath()));
            if(localManager -> hasTexture(id)) {
                drawable.setTexture(localManager -> getTexture(id));
            }
            else {
                if(manager -> hasImage(id)) {
                    auto& image = manager -> getImage(id);
                    auto* texture = image.getSize().x && image.getSize().y
                            ? localManager -> addTexture(id) : nullptr;
                    if(texture) {
                        texture -> create(image);
                        drawable.setTexture(*texture);
                    }
                }
            }

        }

        if(entity.hasComponent<TextComponent>()) {
            auto& text = entity.getComponent<TextComponent>();
            auto id = text.getFontPath().empty() ? std::string{} : resourceFileKey(
                combinePath(m_activeScene -> getAssociatedProjectPath(), text.getFontPath()));
            if(localManager -> hasFont(id)) {
                text.setFont(localManager -> getFont(id));
            }
            else if(manager -> hasFont(id)) {
                auto& font = manager -> getFont(id);
                auto* localFont = !font.getImages().empty()
                        ? localManager -> addFont(id) : nullptr;
                if(localFont) {
                    localFont -> clone(font);
                    text.setFont(*localFont);
                }
            }
        }

        if(entity.hasComponent<AnimationComponent>()) {
            const auto animations = manager -> getAnimationsCopy(entity.getUUID());
            for(const auto& copyAnimation: animations) {
                if(copyAnimation.frames.empty())
                    continue;
                auto* animation = localManager -> addAnimation(entity.getComponent<IDComponent>().ID);
                *animation = copyAnimation;

                auto imageId = resourceFileKey(combinePath(m_activeScene -> getAssociatedProjectPath(),
                        animation -> texturePath));

                if(localManager -> hasTexture(imageId))
                    animation -> texture = &localManager -> getTexture(imageId);
                else {
                    auto& image = manager -> getImage(imageId);
                    auto* localTexture = localManager -> addTexture(imageId);
                    if(localTexture) {
                        localTexture -> create(image);
                        animation -> texture = localTexture;
                    }
                }
            }
            auto& animationComponent = entity.getComponent<AnimationComponent>();
            const auto* localAnimations = localManager -> findAnimations(entity.getUUID());
            if(localAnimations && !localAnimations -> empty())
                animationComponent.setAnimation(&localManager -> getAnimations(entity.getUUID()).front());
        }

        if(entity.hasChildren()) {
            for(auto& child: entity.getChildren())
                processEntity(child);
        }
    }

    void EditorLogic::duplicateEntity(robot2D::vec2f mousePos) {
        std::vector<SceneEntity> duplicatedEntities;
        auto selectedEntities = getSelectedEntities();
        duplicatedEntities.reserve(selectedEntities.size());

        for(const auto& entity : selectedEntities) {
            auto dupEntity = m_activeScene -> duplicateEntity(mousePos, entity);
            auto rect = dupEntity.calculateRect();
            auto treeIterator = m_quadTree.insert(dupEntity, rect);
            if(!dupEntity.hasComponent<QuadTreeComponent>())
                dupEntity.addComponent<QuadTreeComponent>().iterator = treeIterator;
            else
                dupEntity.getComponent<QuadTreeComponent>().iterator = treeIterator;
            duplicatedEntities.emplace_back(dupEntity);
        }

        m_commandStack.addCommand<DuplicateCommand>(m_messageBus, m_activeScene, duplicatedEntities);
    }

    void EditorLogic::addPrefabEntity(const PrefabLoadMessage& message) {
        if(!m_currentProject || !m_activeScene || m_activeScene -> isRunning())
            return;
        PrefabManager manager;
        auto prefab = manager.loadPrefab(m_currentProject -> getPath(), message.path, getScriptInteractor());
        if(!prefab)
            return;
        auto instance = duplicateEmptyEntity(prefab -> getEntity());
        if(!instance) {
            RB_EDITOR_ERROR("Can't instantiate prefab: path='{}'", message.path);
            return;
        }
        m_selectionService.selectEntity(instance.getUUID());
        notifySelection();
    }


    void EditorLogic::onBeginPopup() {
        m_stateBeforePopup = m_presenter.getState();
        m_presenter.switchState(EditorState::Modal);
    }

    void EditorLogic::onEndPopup() {
        if(m_presenter.getState() == EditorState::Modal)
            m_presenter.switchState(m_stateBeforePopup);
    }

    void EditorLogic::undoCommand() {
        m_commandStack.undo();
    }

    void EditorLogic::redoCommand() {
        m_commandStack.redo();
    }


    void EditorLogic::findSelectEntities(const robot2D::FloatRect& rect) {
        std::vector<UUID> selectedEntities;
        auto&& foundItems = m_quadTree.search(rect);
        for(auto iterator: foundItems) {
            auto entity = iterator -> value;
            if(entity)
                selectedEntities.emplace_back(entity.getUUID());
        }
        m_selectionService.selectEntities(selectedEntities);
        notifySelection();
    }


    void EditorLogic::removeEntities(std::vector<SceneEntity>& entities, bool clearContainer) {
        auto restoreInformation = m_activeScene -> removeEntities(entities);
        for(const auto& entity: entities)
            m_selectionService.removeEntity(entity.getUUID());
        auto command =
            m_commandStack.addCommand<DeleteEntitiesCommand>(this, restoreInformation);
        if(!command) {
            RB_EDITOR_ERROR("EditorLogic: Can't Create DeleteCommand");
        }

        if(clearContainer)
            entities.clear();
    }



    void EditorLogic::removeSelectedEntities() {
        constexpr bool clearContainer = true;
        auto selectedEntities = getSelectedEntities();
        if(selectedEntities.empty())
            return;
        removeEntities(selectedEntities, clearContainer);
        m_selectionService.clear();
        m_presenter.clearSelectionOnUI();
    }

    EditorState EditorLogic::getState() const {
        return m_presenter.getState();
    }

    //////////////////////////////////////// UIInteractor ////////////////////////////////////////

    std::vector<SceneEntity> EditorLogic::getSelectedEntities() {
        std::vector<SceneEntity> entities;
        auto selectedEntities = m_selectionService.getSelectedEntities();
        if(!m_activeScene) {
            m_selectionService.clear();
            return entities;
        }
        for(auto uuid: selectedEntities) {
            auto entity = m_activeScene -> getEntity(uuid);
            if(entity)
                entities.emplace_back(entity);
            else
                m_selectionService.removeEntity(uuid);
        }
        return entities;
    }

    SelectionService& EditorLogic::getSelectionService() {
        return m_selectionService;
    }

    void EditorLogic::notifySelection() {
        auto active = m_selectionService.lastActive();
        auto* msg = m_messageBus.postMessage<PanelEntitySelectedMessage>(MessageID::PanelEntitySelected);
        msg -> entity = active && m_activeScene ? m_activeScene -> getEntity(*active) : SceneEntity{};
    }

    std::string EditorLogic::getAssociatedProjectPath() const {
        return m_activeScene -> getAssociatedProjectPath();
    }

    const std::list<SceneEntity>& EditorLogic::getEntities() const {
        return m_activeScene -> getEntities();
    }

    void EditorLogic::addEmptyEntity() {
        auto entity = m_activeScene -> addEmptyEntity();
        auto rect = entity.calculateRect();
        auto treeIterator = m_quadTree.insert(entity, rect);
        entity.addComponent<QuadTreeComponent>().iterator = treeIterator;
    }

    SceneEntity EditorLogic::addButton() {
        auto entity = m_activeScene -> addEmptyButton();
        auto rect = entity.calculateRect();
        auto treeIterator = m_quadTree.insert(entity, rect);
        entity.addComponent<QuadTreeComponent>().iterator = treeIterator;
        return entity;
    }

    SceneEntity EditorLogic::createEmptyEntity() {
        return m_activeScene -> createEmptyEntity();
    }

    SceneEntity EditorLogic::duplicateEmptyEntity(SceneEntity entity) {
        if(!m_activeScene || m_activeScene -> isRunning())
            return {};
        auto instance = m_activeScene -> duplicateEmptyEntity(entity, getScriptInteractor());
        if(instance)
            processEntity(instance);
        return instance;
    }

    bool EditorLogic::setBefore(SceneEntity sourceEntity, SceneEntity target) {
        return m_activeScene -> setBefore(sourceEntity, target);
    }

    bool EditorLogic::reparentEntity(SceneEntity entity, SceneEntity parent) {
        return m_activeScene && m_activeScene -> reparent(entity, parent);
    }

    bool EditorLogic::isRunning() const {
        if(!m_activeScene)
            return false;
        return m_activeScene -> isRunning();
    }

    SceneEntity EditorLogic::findEntity(const robot2D::vec2i& mousePos) {
        robot2D::FloatRect rect = {{ static_cast<float>(mousePos.x),
            static_cast<float>(mousePos.y) }, { 1.f, 1.f }};
        auto&& foundItems = m_quadTree.search(rect);
        std::vector<SceneEntity> entities;
        for(auto iterator: foundItems) {
            auto entity = iterator -> value;
            if(entity && entity.hasComponent<TransformComponent>())
                entities.emplace_back(entity);
        }
        if(entities.empty()) {
            m_selectionService.clear();
            notifySelection();
            return {};
        }
        std::stable_sort(entities.begin(), entities.end(), [](SceneEntity left, SceneEntity right) {
            auto leftDepth = left.hasComponent<DrawableComponent>()
                             ? left.getComponent<DrawableComponent>().getDepth() : 0;
            auto rightDepth = right.hasComponent<DrawableComponent>()
                              ? right.getComponent<DrawableComponent>().getDepth() : 0;
            return leftDepth > rightDepth;
        });
        auto entity = entities.front();
        m_selectionService.selectEntity(entity.getUUID());
        notifySelection();
        return entity;
    }


    bool EditorLogic::hasSelectedEntities() const {
        return !m_selectionService.empty();
    }

    void EditorLogic::restoreDeletedEntities(DeletedEntitiesRestoreInformation& restoreInformation) {
        m_activeScene -> restoreEntities(restoreInformation);
    }


    const std::vector<class_id>& EditorLogic::getCommandStack() const {
        return m_commandStack.getCommandStack();
    }

    void EditorLogic::closeCurrentProject(std::function<void()>&& resultCallback) {
        if(m_presenter.getState() == EditorState::Modal)
            return;
        auto taskQueue = TaskQueue::GetQueue();
        m_closeResultProjectCallback = std::move(resultCallback);
        if((m_activeScene && m_activeScene -> hasChanges()) || m_presenter.hasAssetChanges() ||
            taskQueue -> hasPendingTasks()) {
            m_popupConfiguration.title = "Close Project";
            m_popupConfiguration.error.clear();
            m_popupConfiguration.onYes = [this]() {
                if(!saveAll()) {
                    m_popupConfiguration.error =
                        "Could not save changes. Check the file path and write access, then retry.";
                    RB_EDITOR_ERROR("EditorLogic: can't save changes");
                    return false;
                }
                m_presenter.switchState(EditorState::Load);
                m_closeResultProjectCallback();
                return true;
            };
            m_popupConfiguration.onNo = [this]() {
                m_presenter.discardAssetChanges();
                m_closeResultProjectCallback();
            };
            m_presenter.showPopup(&m_popupConfiguration);
        }
        else
            m_closeResultProjectCallback();
    }

    void EditorLogic::applyPrefabChanges(UUID prefabID, SceneEntity entity) {
        PrefabAssetModificatedMessage message;
        message.prefabUUID = prefabID;
        message.prefabEntity = entity;
        prefabModificated(message);
    }

    void EditorLogic::prefabModificated(const PrefabAssetModificatedMessage& message) {
        if(!m_activeScene || !message.prefabEntity)
            return;
        constexpr auto processModification = [](SceneEntity prefabEntity,
            SceneEntity modificateEntity) {
            if(prefabEntity.hasComponent<DrawableComponent>()
                && modificateEntity.hasComponent<DrawableComponent>()) {
                auto& modificateDrawable = modificateEntity.getComponent<DrawableComponent>();
                auto prefabDrawable = prefabEntity.getComponent<DrawableComponent>();
                modificateDrawable.setColor(prefabDrawable.getColor());
                if(prefabDrawable.hasTexture())
                    modificateDrawable.setTexture(prefabDrawable.getTexture());
                modificateEntity.markModified();
            }
        };

        m_activeScene -> forEachEntity(std::move([&processModification, message](SceneEntity& sceneEntity) {
            if(sceneEntity.hasComponent<PrefabComponent>()) {
                auto& prefabComponent = sceneEntity.getComponent<PrefabComponent>();
                if(prefabComponent.prefabUUID == message.prefabUUID) {
                    processModification(message.prefabEntity, sceneEntity);
                }
            }
        }));
    }

    robot2D::vec2f EditorLogic::getMainCameraPosition() const {
        auto& cameraComponent = m_mainCameraEntity.getComponent<TransformComponent>();
        return cameraComponent.getPosition();
    }

    void EditorLogic::setMainCamera(SceneEntity cameraEntity) {
        if(cameraEntity)
            m_selectionService.selectEntity(cameraEntity.getUUID());
        else
            m_selectionService.clear();
    }

    void EditorLogic::exportProject(const ExportOptions& exportOptions) {
        auto queue = TaskQueue::GetQueue();
        RB_EDITOR_INFO("[Build] Requested; output='{}'; start_scene='{}'", exportOptions.outputFolder,
            exportOptions.startScene);
        auto id = queue -> addAsyncTaskWithCompletion<ExportTask>([options = exportOptions](const ExportTask& task) {
            if(task.getOutcome() != TaskOutcome::Succeeded)
                RB_EDITOR_ERROR("[Build] Not created; output='{}'; start_scene='{}'; reason='{}'",
                options.outputFolder, options.startScene, task.getErrorMessage());
        }, exportOptions);
        if(id == InvalidTaskId)
            RB_EDITOR_ERROR("[Build] Not queued; output='{}'; reason='task queue is stopping'",
                exportOptions.outputFolder);
    }

    SceneEntity EditorLogic::getEntity(UUID uuid) {
        return m_activeScene -> getEntity(uuid);
    }

    SceneEntity EditorLogic::getEntity(const std::string& name) {
        return m_activeScene -> getEntity(name);
    }

    void EditorLogic::setEditorCamera(IEditorCamera::Ptr editorCamera) {
        m_activeScene -> setEditorCamera(editorCamera);
    }

    void EditorLogic::showScriptError(const std::string& diagnostic) {
        RB_EDITOR_ERROR("[ScriptBuild] Failed; project='{}'; reason={}",
            m_currentProject ? m_currentProject -> getPath() : std::string{}, diagnostic);
        m_popupConfiguration = {};
        m_popupConfiguration.title = "Scripts build failed";
        m_popupConfiguration.error = diagnostic.substr(0, 4096) + "\nSee Console for the complete build log.";
        m_presenter.showPopup(&m_popupConfiguration);
    }

    void EditorLogic::generateProject(const GenerateProjectMessage& message) {
        buildScripts(false, std::filesystem::u8path(message.genPath));
    }

    void EditorLogic::buildScripts(bool startRuntime, std::filesystem::path sourceDirectory) {
        if(!m_currentProject || !m_activeScene || m_activeScene -> isRunning() || m_scriptBuildPending) return;
        auto project = m_currentProject;
        auto scene = m_activeScene;
        m_scriptBuildPending = true;
        m_startAfterScriptBuild = startRuntime;
        RB_EDITOR_INFO("[ScriptBuild] Started; project='{}'", project -> getPath());
        auto id = TaskQueue::GetQueue() -> addAsyncTaskWithCompletion<ScriptProjectTask>(
            [this, project, scene](const ScriptProjectTask& task) {
            m_scriptBuildPending = false;
            if(project != m_currentProject || scene != m_activeScene) return;
            if(task.getOutcome() != TaskOutcome::Succeeded) {
                m_startAfterScriptBuild = false;
                showScriptError(task.getErrorMessage()); return;
            }
            auto scripts = m_scriptInteractor.lock();
            auto* engine = scripts ? scripts -> getScriptingEngine() : nullptr;
            if(!engine || !engine -> runtimeInit(task.result.assembly.u8string())) {
                m_startAfterScriptBuild = false;
                showScriptError("The newly compiled script assembly could not be loaded."); return;
            }
            RB_EDITOR_INFO("[ScriptBuild] Succeeded; path='{}'; details={}", task.result.assembly.u8string(),
            task.result.diagnostic);
            if(!m_startAfterScriptBuild) return;
            m_startAfterScriptBuild = false;
            std::string missing;
            m_activeScene -> forEachEntity([&](SceneEntity& entity) {
                if(entity.hasComponent<ScriptComponent>()) {
                    const auto& name = entity.getComponent<ScriptComponent>().name;
                    if(!name.empty() && !engine -> hasEntityClass(name)) missing += name + "\n";
                }
            });
            if(!missing.empty()) {
                showScriptError("Scene script classes are missing from the new assembly:\n" + missing); return;
            }
            m_activeScene -> onRuntimeStart(scripts);
            m_presenter.switchState(EditorState::Run);
        }, project, std::move(sourceDirectory), scriptProjectTools());
        if(id == InvalidTaskId) {
            m_scriptBuildPending = false; m_startAfterScriptBuild = false;
            showScriptError("The task queue is stopping; scripts were not built.");
        }
    }

    std::shared_ptr<IScriptInteractorFrom> EditorLogic::getScriptInteractor() const {
        auto ptr = m_scriptInteractor.lock();
        if(ptr)
            return ptr;
        return nullptr;
    }

    //////////////////////////////////////// UIInteractor ////////////////////////////////////////

} // namespace editor



