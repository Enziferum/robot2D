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

#include <editor/EditorLogic.hpp>
#include <editor/Editor.hpp>
#include <editor/FileApi.hpp>
#include <editor/Messages.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/LocalResourceManager.hpp>
#include <editor/Components.hpp>

#include <editor/components/ButtonComponent.hpp>
#include <editor/scripting/ScriptingEngine.hpp>


#include <editor/async/SceneLoadTask.hpp>
#include <editor/Components.hpp>
#include <editor/commands/DuplicateCommand.hpp>
#include <editor/commands/DeleteEntitiesCommand.hpp>
#include <editor/commands/PasteCommand.hpp>
#include <editor/async/ExportTask.hpp>


#ifdef ROBOT2D_WINDOWS
    #include <windows.h>    //GetModuleFileNameW
#else
    #include <climits>
    #include <unistd.h>     //readlink
#endif




namespace {
    const std::string scenePath = "assets/scenes";

    std::filesystem::path get_exec_path() {
    #ifdef ROBOT2D_WINDOWS
            wchar_t path[MAX_PATH] = { 0 };
            GetModuleFileNameW(NULL, path, MAX_PATH);
            return path;
    #else
            char result[PATH_MAX];
            ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
            return std::string(result, (count > 0) ? count : 0);
    #endif
    }
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

    template<typename T, typename = std::enable_if_t<std::is_pod_v<T>>>
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

    ScriptInteractor::~ScriptInteractor() = default;

    EditorLogic::EditorLogic(robot2D::MessageBus& messageBus,
                             MessageDispatcher& messageDispatcher,
                             EditorPresenter& presenter,
                             EditorRouter& router):
    m_messageBus{messageBus},
    m_messageDispatcher{messageDispatcher},
    m_presenter{presenter},
    m_router{router},
    m_sceneManager{messageBus},
    m_activeScene{nullptr}
    {
        m_messageDispatcher.onMessage<MenuProjectMessage>(MessageID::SaveScene, BIND_CLASS_FN(saveScene));
        m_messageDispatcher.onMessage<ToolbarMessage>(MessageID::ToolbarPressed, BIND_CLASS_FN(toolbarPressed));
        m_messageDispatcher.onMessage<OpenSceneMessage>(MessageID::OpenScene, BIND_CLASS_FN(openScene));
        m_messageDispatcher.onMessage<CreateSceneMessage>(MessageID::CreateScene, BIND_CLASS_FN(createScene));
        m_messageDispatcher.onMessage<PrefabLoadMessage>(MessageID::PrefabLoad, BIND_CLASS_FN(addPrefabEntity));
        m_messageDispatcher.onMessage<PanelEntitySelectedMessage>(MessageID::PanelEntitySelected,
            [this](const PanelEntitySelectedMessage& message) {
            m_selectedEntities.clear();
            m_selectedEntities.emplace_back(message.entity);
        });

        m_messageDispatcher.onMessage<PrefabAssetModificatedMessage>(MessageID::PrefabAssetModificated,
                                                                     BIND_CLASS_FN(prefabModificated));

        m_messageDispatcher.onMessage<AnimationPlayMessage>(MessageID::AnimationPlay,
                                                            [](const AnimationPlayMessage& message){
            auto entity = message.entity;
            auto& animator = entity.getComponent<AnimatorComponent>();
            auto& animationComponent = entity.getComponent<AnimationComponent>();
            if(message.mode == AnimationPlayMessage::Mode::Play)
                animator.Play(animationComponent.getAnimation() -> name);
            else
                animator.Stop(animationComponent.getAnimation() -> name);
        });


        m_messageDispatcher.onMessage<GenerateProjectMessage>(MessageID::GenerateProject,
                                                              BIND_CLASS_FN(generateProject));
    }


    void EditorLogic::handleEventsRuntime(const robot2D::Event& event) {
        m_activeScene -> handleEventsRuntime(event);
    }

    void EditorLogic::update(float dt) {
        auto state = m_presenter.getState();
        switch(state) {
            case EditorState::Load:
                break;
            case EditorState::Edit:
                m_activeScene -> update(dt);
                break;
            case EditorState::Run:
                m_activeScene -> updateRuntime(dt);
                break;
            default:
                break;
        }
    }

    void EditorLogic::destroy() {
        TaskQueue::GetQueue() -> stop();
    }

    void EditorLogic::createProject(Project::Ptr project) {
        m_currentProject = project;
        if(!m_sceneManager.add(std::move(project))) {
            RB_EDITOR_ERROR("Can't Create Scene. Reason: {0}",
                            errorToString(m_sceneManager.getError()));
            return;
        }
        m_activeScene = m_sceneManager.getActiveScene();

        m_activeScene -> createMainCamera();
        m_mainCameraEntity = m_activeScene -> getEntities().back();
        m_presenter.setMainCameraEntity(m_mainCameraEntity);
        m_router.openScene(m_activeScene, m_currentProject -> getPath());
    }

    void EditorLogic::loadProject(Project::Ptr project) {
        m_currentProject = project;
        m_presenter.switchState(EditorState::Load);

        auto loadLambda = [](const SceneLoadTask& task) {
            task.logic -> loadSceneCallback();
        };

        auto path = project -> getPath();
        auto appendPath = combinePath(scenePath, project -> getStartScene());
        auto scenePath = combinePath(path, appendPath);

        m_presenter.prepareView();
        TaskQueue::GetQueue() -> addAsyncTask<SceneLoadTask>(loadLambda, m_sceneManager, project, scenePath, this);
    }

    /// TODO(a.raag): if scene don't have entities create main camera ???
    void EditorLogic::openScene(const OpenSceneMessage& message) {
        auto path = m_currentProject -> getPath();
        auto scenePath = combinePath(path, message.path);
        auto currentScenePath = combinePath(path, m_activeScene -> getPath());
        if(std::filesystem::path(scenePath) == std::filesystem::path(currentScenePath))
            return;

        auto taskQueue =  TaskQueue::GetQueue();
        if(m_activeScene -> hasChanges() || taskQueue -> hasPendingTasks()) {
            m_popupConfiguration.title = "Close Scene";
            m_popupConfiguration.onYes = [this, taskQueue, scenePath]() {
                m_presenter.switchState(EditorState::Load);
                if(!saveScene()) {
                    RB_EDITOR_ERROR("EditorLogic: can't save scene");
                }

                auto loadLambda = [](const SceneLoadTask& task) {
                    task.logic -> loadSceneCallback();
                };

                taskQueue -> addAsyncTask<SceneLoadTask>(loadLambda, m_sceneManager,
                                                                     m_currentProject, scenePath, this);
            };

            m_popupConfiguration.onNo = [this, taskQueue, scenePath]() {
                m_presenter.switchState(EditorState::Load);
                auto loadLambda = [](const SceneLoadTask& task) {
                    task.logic -> loadSceneCallback();
                };

                taskQueue -> addAsyncTask<SceneLoadTask>(loadLambda, m_sceneManager,
                                                         m_currentProject, scenePath, this);
            };
            m_presenter.showPopup(&m_popupConfiguration);
        }
        else {
            m_presenter.switchState(EditorState::Load);
            auto loadLambda = [](const SceneLoadTask& task) {
                task.logic -> loadSceneCallback();
            };

            TaskQueue::GetQueue() -> addAsyncTask<SceneLoadTask>(loadLambda, m_sceneManager,
                                                                 m_currentProject, scenePath, this);
        }
    }

    void EditorLogic::createScene(const CreateSceneMessage& message) {
        if(!m_sceneManager.add(std::move(m_currentProject), message.path)) {
            RB_EDITOR_ERROR("Can't Create Scene. Reason: {0}",
                            errorToString(m_sceneManager.getError()));
            return;
        }
    }

    void EditorLogic::saveScene(const MenuProjectMessage& message) {
        m_sceneManager.save(std::move(m_activeScene));
    }

    bool EditorLogic::saveScene() {
        return m_sceneManager.save(std::move(m_activeScene));
    }

    void EditorLogic::loadSceneCallback() {
        m_presenter.switchState(EditorState::Edit);
        m_activeScene = m_sceneManager.getActiveScene();
        m_presenter.setMainCameraEntity({});

        for(auto& entity: m_activeScene -> getEntities()) {
            loadAssetsByEntity(entity);
            if(entity.hasComponent<CameraComponent>()) {
                auto& cameraComponent = entity.getComponent<CameraComponent>();
                if(cameraComponent.isPrimary) {
                    m_mainCameraEntity = entity;
                    m_presenter.setMainCameraEntity(m_mainCameraEntity);
                }
            }

            if(entity.hasComponent<ButtonComponent>()) {
                auto& btnComp = entity.getComponent<ButtonComponent>();
                if(!btnComp.onClickCallback) {
                    m_activeScene -> registerUICallback(entity);

                    btnComp.onClickCallback = [](UUID scriptUUID, const std::string& methodName) {
                        auto instance = ScriptEngine::getEntityScriptInstance(scriptUUID);
                        if(instance)
                            instance -> getClassWrapper() -> callMethod(methodName);
                    };
                }
            }

            auto& ts = entity.getComponent<TransformComponent>();
            for(auto child: ts.getChildren())
                loadAssetsByEntity(child);
        }

        auto filename = std::filesystem::path{m_activeScene -> getPath()}.filename().string();
        m_currentProject -> setStartScene(filename);

        // TODO(a.raag): update project's start scene
        //m_currentProject -> save();
        m_router.openScene(m_activeScene, m_currentProject -> getPath());
    }


    void EditorLogic::copyToBuffer() {
        m_copyEntities.clear();
        m_copyEntities = m_selectedEntities;
    }

    void EditorLogic::pasterFromBuffer() {
        std::vector<robot2D::ecs::Entity> copiedEntities{};
        int counter = 0;

        struct PasteInfo {
            bool isParentPasted{false};
            robot2D::ecs::Entity entity;
        };

        for(auto& copy: m_copyEntities) {
            auto& transform = copy.getComponent<TransformComponent>();
            auto copiedEntity = m_activeScene -> duplicateEntity(transform.getPosition(), copy);

            if(transform.hasChildren())
                pasteChild(copy);

            copiedEntities.emplace_back(copiedEntity);
        }

        auto command = m_commandStack.addCommand<PasteCommand>(m_messageBus, copiedEntities, this);
        if(!command) {
            RB_EDITOR_ERROR("EditorLogic: Can't Create PasteCommand");
        }

        /// m_selectedEntities = copiedEntities;
        /// ui create
    }


    void EditorLogic::pasteChild(robot2D::ecs::Entity parent) {}


    void EditorLogic::toolbarPressed(const ToolbarMessage& message) {
        if(message.pressedType == 1) {
            m_presenter.switchState(EditorState::Run);
            m_activeScene -> onRuntimeStart(this);
        }
        else if(message.pressedType == 0) {
            m_activeScene -> onRuntimeStop();
            m_presenter.switchState(EditorState::Edit);
        }
    }

    void EditorLogic::loadAssetsByEntity(robot2D::ecs::Entity entity) {
        if(entity.hasComponent<ButtonComponent>()) {
            auto& btnComp = entity.getComponent<ButtonComponent>();
            if(!btnComp.onClickCallback) {
                m_activeScene -> registerUICallback(entity);

                btnComp.onClickCallback = [](UUID scriptUUID, const std::string& methodName) {
                    auto instance = ScriptEngine::getEntityScriptInstance(scriptUUID);
                    if(instance)
                        instance -> getClassWrapper() -> callMethod(methodName);
                };
            }
        }

        if(!entity.hasComponent<DrawableComponent>())
            return;

        auto* manager = ResourceManager::getManager();
        auto* localManager = LocalResourceManager::getManager();

        auto& drawable = entity.getComponent<DrawableComponent>();
        std::filesystem::path texturePath{ drawable.getTexturePath() };
        auto id = texturePath.filename().string();
        if(localManager -> hasTexture(id)) {
            drawable.setTexture(localManager -> getTexture(id));
        }
        else {
            if(manager -> hasImage(id)) {
                auto& image = manager -> getImage(id);
                auto* texture = localManager -> addTexture(texturePath.filename().string());
                if(texture) {
                    texture -> create(image);
                    drawable.setTexture(*texture);
                }
            }
        }

        if(entity.hasComponent<TextComponent>()) {
            auto& text = entity.getComponent<TextComponent>();
            std::filesystem::path fontPath{ text.getFontPath() };
            auto id = fontPath.filename().string();
            if(localManager -> hasFont(id)) {
                text.setFont(localManager -> getFont(id));
            }
            else {
                if(!manager -> hasFont(id))
                    return;
                auto& font = manager -> getFont(id);
                auto* localFont = localManager -> addFont(fontPath.filename().string());
                if(localFont) {
                    localFont -> clone(font);
                    text.setFont(*localFont);
                }
            }
        }

        if(entity.hasComponent<AnimationComponent>()) {
            for(auto& copyAnimation: manager -> getAnimations(entity.getComponent<IDComponent>().ID)) {
                auto* animation = localManager -> addAnimation(entity.getComponent<IDComponent>().ID);
                *animation = copyAnimation;

                std::filesystem::path imagePath { animation -> texturePath };
                auto imageId = imagePath.filename().string();

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
            animationComponent.setAnimation(
                    &localManager -> getAnimations(entity.getComponent<IDComponent>().ID)[0]);
        }
    }

    void EditorLogic::notifyObservers(std::vector<std::string>&& paths) {
        for(auto& observer: m_observers)
            observer -> notify(std::move(paths));
    }

    void EditorLogic::addObserver(Observer::Ptr observer) {
        m_observers.emplace_back(observer);
    }

    void EditorLogic::duplicateEntity(robot2D::vec2f mousePos, robot2D::ecs::Entity entity) {
        auto dupEntity = m_activeScene -> duplicateEntity(mousePos, entity);
        m_commandStack.addCommand<DuplicateCommand>(m_messageBus, m_activeScene, dupEntity);
    }

    void EditorLogic::addPrefabEntity(const PrefabLoadMessage& message) {
        auto path = m_currentProject -> getPath();
        auto prefabPath = combinePath(path, message.path);
    }


    void EditorLogic::onBeginPopup() {
         m_presenter.switchState(EditorState::Modal);
    }

    void EditorLogic::onEndPopup() {
        m_presenter.switchState(EditorState::Edit);
    }

    void EditorLogic::undoCommand() {
        m_commandStack.undo();
    }

    void EditorLogic::redoCommand() {
        m_commandStack.redo();
    }


    void EditorLogic::findSelectEntities(const robot2D::FloatRect& rect) {
        m_selectedEntities.clear();

        for(auto& entity: m_activeScene -> getEntities()) {
            auto& transform = entity.getComponent<TransformComponent>();
            if(rect.contains(transform.getGlobalBounds()))
                m_selectedEntities.emplace_back(entity);

            if(transform.hasChildren())
                findSelectChildren(rect, entity);
        }

        if(!m_selectedEntities.empty()) {
            std::vector<ITreeItem::Ptr> selected_items{};
            selected_items.reserve(m_selectedEntities.size());

            for(auto& entity: m_selectedEntities) {
                auto& component = entity.getComponent<UIComponent>();
                selected_items.push_back(component.treeItem);
            }
            m_presenter.findSelectedEntitiesOnUI(std::move(selected_items));

            if(m_selectedEntities.size() == 1) {
                auto* msg=
                        m_messageBus.postMessage<PanelEntitySelectedMessage>(MessageID::PanelEntityNeedSelect);
                msg -> entity = m_selectedEntities.back();
            }

        }

        /// TODO(a.raag): add Selection Command
    }

    void EditorLogic::findSelectChildren(const robot2D::FloatRect& rect, robot2D::ecs::Entity entity) {
        auto& transform = entity.getComponent<TransformComponent>();
        for(auto& child: transform.getChildren()) {
            if(!child || child.destroyed())
                continue;
            auto& childTransform = child.getComponent<TransformComponent>();
            if(rect.contains(childTransform.getGlobalBounds()))
                m_selectedEntities.emplace_back(child);
            if(childTransform.hasChildren())
                findSelectChildren(rect, child);
        }
    }

    void EditorLogic::removeSelectedEntities() {
        auto restoreInformation = m_activeScene -> removeEntities(m_selectedEntities);

        std::vector<ITreeItem::Ptr> uiItems;
        uiItems.reserve(m_selectedEntities.size());

        for(auto& entity: m_selectedEntities) {
            if(entity && !entity.destroyed())
                uiItems.emplace_back(entity.getComponent<UIComponent>().treeItem);
        }

        auto uiDeletedInformation = m_presenter.removeEntitiesFromUI(std::move(uiItems));

        auto command =
                m_commandStack.addCommand<DeleteEntitiesCommand>(this, restoreInformation, uiDeletedInformation);
        if(!command) {
            RB_EDITOR_ERROR("EditorLogic: Can't Create DeleteCommand");
        }

        m_selectedEntities.clear();
    }

    robot2D::ecs::Entity EditorLogic::getByUUID(std::uint64_t uuid) {
        return m_activeScene -> getByUUID(UUID(uuid));
    }

    EditorState EditorLogic::getState() const {
        return m_presenter.getState();
    }

    //////////////////////////////////////// UIInteractor ////////////////////////////////////////

    std::vector<robot2D::ecs::Entity>& EditorLogic::getSelectedEntities()  {
        return m_selectedEntities;
    }

    std::string EditorLogic::getAssociatedProjectPath() const {
        return m_activeScene -> getAssociatedProjectPath();
    }

    std::list<robot2D::ecs::Entity> EditorLogic::getEntities() const {
        return m_activeScene -> getEntities();
    }

    void EditorLogic::removeEntity(robot2D::ecs::Entity entity) {
        m_activeScene -> removeEntity(entity);
    }

    void EditorLogic::addEmptyEntity() {
        m_activeScene -> addEmptyEntity();
    }

    robot2D::ecs::Entity EditorLogic::addButton() {
        return m_activeScene -> addEmptyButton();
    }

    robot2D::ecs::Entity EditorLogic::createEmptyEntity() {
        return m_activeScene -> createEmptyEntity();
    }

    robot2D::ecs::Entity EditorLogic::duplicateEmptyEntity(robot2D::ecs::Entity entity) {
        return m_activeScene -> duplicateEmptyEntity(entity);
    }

    void EditorLogic::setBefore(robot2D::ecs::Entity sourceEntity, robot2D::ecs::Entity target) {
        m_activeScene -> setBefore(sourceEntity, target);
    }

    void EditorLogic::removeEntityChild(robot2D::ecs::Entity entity) {
        m_activeScene -> removeEntityChild(entity);
    }

    bool EditorLogic::isRunning() const {
        if(!m_activeScene)
            return false;
        return m_activeScene -> isRunning();
    }

    robot2D::ecs::Entity EditorLogic::getByUUID(UUID uuid) {
        return m_activeScene -> getByUUID(uuid);
    }

    void EditorLogic::registerOnDeleteFinish(std::function<void()>&& callback) {
        m_activeScene -> registerOnDeleteFinish(std::move(callback));
    }

    robot2D::ecs::Entity EditorLogic::getSelectedEntity(int graphicsEntityID) {
        m_selectedEntities.clear();
        auto entities = m_activeScene -> getEntities();
        for(auto& entity: entities) {
            if(entity.getIndex() == graphicsEntityID) {
                m_selectedEntities.emplace_back(entity);
                auto* msg =
                        m_messageBus.postMessage<PanelEntitySelectedMessage>(MessageID::PanelEntityNeedSelect);
                msg -> entity = entity;
                return entity;
            }


            auto& tx = entity.getComponent<TransformComponent>();
            if(tx.hasChildren()) {
                robot2D::ecs::Entity childEntity = getSelectedEntityChild(entity, graphicsEntityID);
                if(childEntity && !childEntity.destroyed()) {
                    return childEntity;
                }
            }
        }

        if(m_selectedEntities.empty())
            m_presenter.clearSelectionOnUI();

        return robot2D::ecs::Entity{};
    }


    robot2D::ecs::Entity EditorLogic::getSelectedEntityChild(robot2D::ecs::Entity parent, int graphicsEntityID) {
        auto& tx = parent.getComponent<TransformComponent>();
        for(auto& child: tx.getChildren()) {
            if(!child || child.destroyed())
                continue;
            if(child.getIndex() == graphicsEntityID) {
                m_selectedEntities.emplace_back(child);
                auto* msg =
                        m_messageBus.postMessage<PanelEntitySelectedMessage>(MessageID::PanelEntityNeedSelect);
                msg -> entity = child;
                return child;
            }

            auto& childTransform = child.getComponent<TransformComponent>();
            if(childTransform.hasChildren())
                return getSelectedEntityChild(child, graphicsEntityID);
        }
        return robot2D::ecs::Entity{};
    }


    bool EditorLogic::hasSelectedEntities() const {
        return !m_selectedEntities.empty();
    }

    void EditorLogic::restoreDeletedEntities(DeletedEntitiesRestoreInformation& restoreInformation,
                                             DeletedEntitiesRestoreUIInformation& restoreUiInformation) {
        m_activeScene -> restoreEntities(restoreInformation);
        m_presenter.restoreEntitiesOnUI(restoreUiInformation);
    }


    const std::vector<class_id>& EditorLogic::getCommandStack() const {
        return m_commandStack.getCommandStack();
    }

    void EditorLogic::closeCurrentProject(std::function<void()>&& resultCallback) {
        auto taskQueue = TaskQueue::GetQueue();
        m_closeResultProjectCallback = std::move(resultCallback);
        if(m_activeScene -> hasChanges() || taskQueue -> hasPendingTasks()) {
            m_popupConfiguration.title = "Close Project";
            m_popupConfiguration.onYes = [this]() {
                m_presenter.switchState(EditorState::Load);
                if(!saveScene()) {
                    RB_EDITOR_ERROR("EditorLogic: can't save scene");
                }
                m_closeResultProjectCallback();
            };
            m_popupConfiguration.onNo = [this]() {
                m_closeResultProjectCallback();
            };
            m_presenter.showPopup(&m_popupConfiguration);
        }
        else
            m_closeResultProjectCallback();
    }

    void EditorLogic::prefabModificated(const PrefabAssetModificatedMessage& message) {
        constexpr auto processModification = [](robot2D::ecs::Entity prefabEntity,
                robot2D::ecs::Entity modificateEntity) {
            if(prefabEntity.hasComponent<DrawableComponent>()
                    && modificateEntity.hasComponent<DrawableComponent>()) {
                auto& modificateDrawable = modificateEntity.getComponent<DrawableComponent>();
                auto prefabDrawable = prefabEntity.getComponent<DrawableComponent>();
                modificateDrawable.setColor(prefabDrawable.getColor());
                modificateDrawable.setTexture(prefabDrawable.getTexture());
            }
        };

        auto& entities = m_activeScene -> getEntities();
        for(auto entity: entities) {
            if(entity.hasComponent<PrefabComponent>()) {
                auto& prefabComponent = entity.getComponent<PrefabComponent>();
                if(prefabComponent.prefabUUID == message.prefabUUID) {
                    processModification(message.prefabEntity, entity);
                }
            }
            if(entity.getComponent<TransformComponent>().hasChildren()) {
                for(auto child: entity.getComponent<TransformComponent>().getChildren()) {
                    if(child.hasComponent<PrefabComponent>()) {
                        auto& prefabComponent = child.getComponent<PrefabComponent>();
                        if(prefabComponent.prefabUUID == message.prefabUUID) {
                            processModification(message.prefabEntity, child);
                        }
                    }
                }
            }
        }
    }

    robot2D::vec2f EditorLogic::getMainCameraPosition() const {
        auto& cameraComponent = m_mainCameraEntity.getComponent<TransformComponent>();
        return cameraComponent.getPosition();
    }

    void EditorLogic::setMainCamera(robot2D::ecs::Entity cameraEntity) {
        m_selectedEntities.clear();
        m_selectedEntities.emplace_back(cameraEntity);
    }

    void EditorLogic::exportProject(const ExportOptions& exportOptions) {
        auto queue = TaskQueue::GetQueue();
        queue -> addAsyncTask<ExportTask>([](const ExportTask& exportTask) {

        }, exportOptions);
    }

    void EditorLogic::uiSelectedEntities(std::set<ITreeItem::Ptr>& uiItems, bool isAll) {
        m_selectedEntities.clear();
        auto& entities = m_activeScene -> getEntities();


        for(auto& uiItem: uiItems) {
           // auto& entity = m_activeScene -> getEntity(uiItem -> getSearchPath());
           // m_selectedEntities.push_back(entity);
        }


    }


    void EditorLogic::setEditorCamera(IEditorCamera::Ptr editorCamera) {
        m_activeScene -> setEditorCamera(editorCamera);
    }

    bool EditorLogic::loadSceneRuntime(std::string&& name) {
        RB_EDITOR_WARN("EditorLogic::loadSceneRuntime Not Implemented Method, Want Load Scene by Name {0}", name);
        /*
         * Steps: TODO(a.raag)
         *  - LoadScene
         *  - Recreate Physics, ScriptEngine
         *  - Update UI
         *  - Command Stack ??
         */

        return true;
    }

    void EditorLogic::loadSceneAsyncRuntime(std::string&& name) {
        RB_EDITOR_WARN("EditorLogic::loadSceneRuntime Not Implemented Method, Want Load Scene by Name {0}", name);
        // TODO(a.raag): work on algorithm in future
    }

    void EditorLogic::exitEngineRuntime() {
        /// TODO(a.raag): check param some where
        constexpr bool insideEngine = true;
        if(insideEngine) {
            /// TODO(a.raag): if runtime scene != default Scene reload defaultScene
            m_activeScene -> onRuntimeStop();
            m_presenter.switchState(EditorState::Edit);
        }
    }

    robot2D::ecs::Entity EditorLogic::duplicateRuntime(robot2D::ecs::Entity entity, robot2D::vec2f position) {
        return m_activeScene -> duplicateRuntime(entity, position);
    }

    void EditorLogic::generateProject(const GenerateProjectMessage& message) {
        std::string genCmd;
#ifdef ROBOT2D_WINDOWS
        genCmd = "python res/gen/gen_project.py";
#else
        genCmd = "python3 res/gen/gen_project.py";
#endif
        genCmd += " --path " + message.genPath;
        genCmd += " --name " + m_currentProject -> getName();
        genCmd += " --copypath " + m_currentProject -> getPath() + "/assets/scripts/bin/";
        auto exePath = get_exec_path();
        exePath = exePath.remove_filename();
        genCmd += " --coredllpath " + exePath.string() + "res/script/robot2D_ScriptCore.dll";

        RB_EDITOR_INFO("EditorLogic: command to generate robot2D's script's project {0}", genCmd);

        /// TODO(a.raag): GenerateProjectTask + Popup
        auto f= std::async([genCmd](){
            if(std::system(genCmd.c_str())) {
                /// nice
            }
            else {
                /// TODO(a.raag): ShowError
            }
        });
    }


    //////////////////////////////////////// UIInteractor ////////////////////////////////////////

} // namespace editor



