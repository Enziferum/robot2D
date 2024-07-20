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
#include <editor/Editor.hpp>
#include <editor/FileApi.hpp>
#include <editor/Messages.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/LocalResourceManager.hpp>
#include <editor/Components.hpp>

#include <editor/components/ButtonComponent.hpp>
#include <editor/scripting/ScriptingEngine.hpp>


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
    const robot2D::FloatRect initRectangle = {{-10000, -10000}, {20000, 20000}};

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
                                                            [](const AnimationPlayMessage& message) {
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

        m_quadTree.resizeRectangle(initRectangle);
    }


    void EditorLogic::handleEventsRuntime(const robot2D::Event& event) {
        m_activeScene -> handleEventsRuntime(event);
    }

    void EditorLogic::update(float dt) {
        auto state = m_presenter.getState();
        switch(state) {
            case EditorState::Load:
                break;
            case EditorState::Edit: {
                for(auto& entity: m_selectedEntities) {
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
        m_scriptInteractor = scriptInteractor;
        PopupManager::getManager() -> addObserver(this);
    }

    void EditorLogic::destroy() {
        TaskQueue::GetQueue() -> stop();
    }

    void EditorLogic::createProject(Project::Ptr project) {
        m_currentProject = project;
        if(!m_sceneManager.add(std::move(project), m_scriptInteractor)) {
            RB_EDITOR_ERROR("Can't Create Scene. Reason: {0}",
                            errorToString(m_sceneManager.getError()));
            return;
        }
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
        m_currentProject = project;
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

        auto taskQueue =  TaskQueue::GetQueue();
        if(m_activeScene -> hasChanges() || taskQueue -> hasPendingTasks()) {
            m_popupConfiguration.title = "Close Scene";
            m_popupConfiguration.onYes = [this, taskQueue, scenePath]() {
                m_presenter.switchState(EditorState::Load);
                if(!saveScene()) {
                    RB_EDITOR_ERROR("EditorLogic: can't save scene");
                }

                m_sceneManager.loadSceneAsync(m_currentProject, scenePath,
                                              BIND_CLASS_FN(loadSceneCallback), m_scriptInteractor);
            };

            m_popupConfiguration.onNo = [this, taskQueue, scenePath]() {
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
        m_sceneManager.save(std::move(m_activeScene), m_scriptInteractor);
    }

    bool EditorLogic::saveScene() {
        return m_sceneManager.save(std::move(m_activeScene), m_scriptInteractor);
    }

    void EditorLogic::loadSceneCallback(Scene::Ptr loadedScene) {
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
    }


    void EditorLogic::copyToBuffer() {
        m_copyEntities.clear();
        m_copyEntities = m_selectedEntities;
    }

    void EditorLogic::pasterFromBuffer() {
        std::vector<SceneEntity> copiedEntities{};
        int counter = 0;

        struct PasteInfo {
            bool isParentPasted{false};
            SceneEntity entity;
        };

        /// child, child -> ???

        for(auto copy: m_copyEntities) {
            auto& transform = copy.getComponent<TransformComponent>();
            auto copiedEntity = m_activeScene -> duplicateEntity(transform.getPosition(), copy);
            copiedEntities.emplace_back(copiedEntity);
            /// ?
            // m_selectedEntities.emplace_back(copiedEntity);
        }

        auto command = m_commandStack.addCommand<PasteCommand>(m_messageBus, std::move(copiedEntities), this);
        if(!command) {
            RB_EDITOR_ERROR("EditorLogic: Can't Create PasteCommand");
        }
    }


    void EditorLogic::pasteChild(SceneEntity parent) {}


    void EditorLogic::switchRuntimeState(const ToolbarMessage::Type& type) {
        auto ptr = m_scriptInteractor.lock();

        if(type == ToolbarMessage::Type::Start) {
            m_activeScene -> onRuntimeStart(ptr);
            m_presenter.switchState(EditorState::Run);
        }
        else if(type == ToolbarMessage::Type::Stop) {
            m_activeScene -> onRuntimeStop(ptr);
            m_presenter.switchState(EditorState::Edit);
        }
    }

    void EditorLogic::processEntity(SceneEntity entity) {
        auto iterator = m_quadTree.insert(entity, entity.calculateRect());
        entity.addComponent<QuadTreeComponent>().iterator = iterator;

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
            animationComponent.setAnimation( &localManager -> getAnimations(entity.getComponent<IDComponent>().ID)[0]);
        }

        if(entity.hasChildren()) {
            for(auto& child: entity.getChildren())
                processEntity(child);
        }
    }

    void EditorLogic::duplicateEntity(robot2D::vec2f mousePos) {
        std::vector<SceneEntity> duplicatedEntities;
        duplicatedEntities.reserve(m_selectedEntities.size());

        for (const auto& entity : m_selectedEntities) {
            auto dupEntity = m_activeScene -> duplicateEntity(mousePos, entity);
            duplicatedEntities.emplace_back(dupEntity);
        }

        m_commandStack.addCommand<DuplicateCommand>(m_messageBus, m_activeScene, duplicatedEntities);
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

        auto&& foundItems = m_quadTree.search(rect);
        m_selectedEntities.resize(foundItems.size());
        std::transform(foundItems.begin(), foundItems.end(), m_selectedEntities.begin(), [](
                const QuadTreeContainer<SceneEntity>::ContainerIterator iterator) {
           return iterator -> value;
        });

        if(!m_selectedEntities.empty()) {
            std::vector<ITreeItem::Ptr> selected_items{};
            selected_items.reserve(m_selectedEntities.size());

            for(auto& entity: m_selectedEntities) {
                auto& component = entity.getComponent<UIComponent>();
                selected_items.push_back(component.treeItem);
            }
            m_presenter.findSelectedEntitiesOnUI(std::move(selected_items));
        }

        /// TODO(a.raag): add Selection Command
    }


    void EditorLogic::removeEntities(std::vector<SceneEntity>& entities, bool clearContainer) {
        auto restoreInformation = m_activeScene -> removeEntities(entities);

        std::list<ITreeItem::Ptr> uiItems;
        for(const auto& entity: entities) {
            if(entity)
                uiItems.emplace_back(entity.getComponent<UIComponent>().treeItem);
        }

        auto uiDeletedInformation = m_presenter.removeEntitiesFromUI(std::move(uiItems));

        auto command =
                m_commandStack.addCommand<DeleteEntitiesCommand>(this, restoreInformation, uiDeletedInformation);
        if(!command) {
            RB_EDITOR_ERROR("EditorLogic: Can't Create DeleteCommand");
        }

        if(clearContainer)
            entities.clear();
    }



    void EditorLogic::removeSelectedEntities() {
        constexpr bool clearContainer = true;
        removeEntities(m_selectedEntities, clearContainer);
    }

    EditorState EditorLogic::getState() const {
        return m_presenter.getState();
    }

    //////////////////////////////////////// UIInteractor ////////////////////////////////////////

    std::vector<SceneEntity>& EditorLogic::getSelectedEntities()  {
        return m_selectedEntities;
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
        return m_activeScene -> duplicateEmptyEntity(entity);
    }

    bool EditorLogic::setBefore(SceneEntity sourceEntity, SceneEntity target) {
        return m_activeScene -> setBefore(sourceEntity, target);
    }

    void EditorLogic::removeEntityChild(SceneEntity entity) {
        m_activeScene -> removeEntityChild(entity);
    }

    bool EditorLogic::isRunning() const {
        if(!m_activeScene)
            return false;
        return m_activeScene -> isRunning();
    }

    SceneEntity EditorLogic::findEntity(const robot2D::vec2i& mousePos) {
        m_selectedEntities.clear();
        robot2D::FloatRect rect = {{ static_cast<float>(mousePos.x),
                                     static_cast<float>(mousePos.y) }, { 1.f, 1.f }};
        auto&& foundItems = m_quadTree.search(rect);
        if(foundItems.empty()) {
            m_presenter.clearSelectionOnUI();
            return {};
        }
        else if(foundItems.size() > 1) {
            std::vector<SceneEntity> tmp;
            tmp.reserve(foundItems.size());
            std::transform(foundItems.begin(), foundItems.end(), tmp.begin(), [](
                    const QuadTreeContainer<SceneEntity>::ContainerIterator iterator) {
                return iterator -> value;
            });

            std::sort(tmp.begin(), tmp.end(), [](SceneEntity entity1, SceneEntity entity2) {
                auto depth1 = entity1.getComponent<DrawableComponent>().getDepth();
                auto depth2 = entity2.getComponent<DrawableComponent>().getDepth();
                return depth1 > depth2;
            });

            m_selectedEntities.push_back(tmp[0]);
            return tmp[0];
        }
        else {
            std::transform(foundItems.begin(), foundItems.end(), m_selectedEntities.begin(), [](
                    const QuadTreeContainer<SceneEntity>::ContainerIterator iterator) {
                return iterator -> value;
            });

            return m_selectedEntities[0];
        }

        return SceneEntity{};
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
        m_quadTree.clear();

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
        constexpr auto processModification = [](SceneEntity prefabEntity,
                SceneEntity modificateEntity) {
            if(prefabEntity.hasComponent<DrawableComponent>()
                    && modificateEntity.hasComponent<DrawableComponent>()) {
                auto& modificateDrawable = modificateEntity.getComponent<DrawableComponent>();
                auto prefabDrawable = prefabEntity.getComponent<DrawableComponent>();
                modificateDrawable.setColor(prefabDrawable.getColor());
                modificateDrawable.setTexture(prefabDrawable.getTexture());
            }
        };

        m_activeScene -> traverseGraph(std::move([&processModification, message](SceneEntity& sceneEntity) {
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

        for(auto& uiItem: uiItems) {
           auto* entity = uiItem -> getUserData<SceneEntity>();
           if(entity && *entity)
               m_selectedEntities.push_back(*entity);
        }
    }

    SceneEntity EditorLogic::getEntity(UUID uuid) {
        return m_activeScene -> getEntity(uuid);
    }

    void EditorLogic::setEditorCamera(IEditorCamera::Ptr editorCamera) {
        m_activeScene -> setEditorCamera(editorCamera);
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
        auto f = std::async([genCmd](){
            if(std::system(genCmd.c_str())) {
                /// nice
            }
            else {
                /// TODO(a.raag): ShowError
            }
        });
    }

    std::shared_ptr<IScriptInteractorFrom> EditorLogic::getScriptInteractor() const {
        auto ptr = m_scriptInteractor.lock();
        if(ptr)
            return ptr;
        return nullptr;
    }

    //////////////////////////////////////// UIInteractor ////////////////////////////////////////

} // namespace editor



