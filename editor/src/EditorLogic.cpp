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

#include <editor/async/SceneLoadTask.hpp>
#include <editor/Components.hpp>
#include <editor/commands/DuplicateCommand.hpp>
#include <editor/commands/DeleteEntitiesCommand.hpp>
#include "editor/commands/PasteCommand.hpp"

namespace {
    const std::string scenePath = "assets/scenes";
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

    void EditorLogic::createProject(Project::Ptr project) {
        m_currentProject = project;
        if(!m_sceneManager.add(std::move(project))) {
            RB_EDITOR_ERROR("Can't Create Scene. Reason: {0}",
                            errorToString(m_sceneManager.getError()));
            return;
        }
        m_activeScene = m_sceneManager.getActiveScene();
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

        for(auto& entity: m_activeScene -> getEntities()) {
            loadAssetsByEntity(entity);
            auto& ts = entity.getComponent<TransformComponent>();
            for(auto child: ts.getChildren())
                loadAssetsByEntity(child);
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
        std::vector<robot2D::ecs::Entity> copiedEntites{int(m_copyEntities.size())};
        int counter = 0;

        for(auto& copy: m_copyEntities) {
            const auto& copyPosition = copy.getComponent<TransformComponent>().getPosition();
            auto copiedEntity = m_activeScene -> duplicateEntity(copyPosition, copy);
            copiedEntites[counter] = copiedEntity;
            ++counter;
        }

        auto command = m_commandStack.addCommand<PasteCommand>(m_messageBus, copiedEntites, this);
        if(!command) {
            RB_EDITOR_ERROR("EditorLogic: Can't Create PasteCommand");
        }

        m_selectedEntities = copiedEntites;
        if(m_selectedEntities.size() == 1) {
            auto* msg =
                    m_messageBus.postMessage<PanelEntitySelectedMessage>(MessageID::PanelEntityNeedSelect);
            msg -> entity = m_selectedEntities[0];
        }
        else {
            // TODO(a.raag): multiply selection
        }
    }


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
            if(!manager -> hasImage(id))
                return;
            auto& image = manager -> getImage(id);
            auto* texture = localManager -> addTexture(texturePath.filename().string());
            if(texture) {
                texture -> create(image);
                drawable.setTexture(*texture);
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

        for(auto& entity: m_activeScene -> getEntities()) {
            auto& transform = entity.getComponent<TransformComponent>();
            if(rect.contains(transform.getGlobalBounds())) {
                m_selectedEntities.emplace_back(entity);
            }

            for(auto& child: transform.getChildren()) {
                if(rect.contains(transform.getGlobalBounds())) {
                    m_selectedEntities.emplace_back(entity);
                }
            }
        }

        m_presenter.findSelectedEntitiesOnUI(m_selectedEntities);

        /// TODO(a.raag): add selection command
        /// m_commandStack.addCommand();
        /// fill m_selectedEntities //
    }

    void EditorLogic::removeSelectedEntities() {
        auto restoreInformation = m_activeScene -> removeEntities(m_selectedEntities);
        auto uiDeletedInformation = m_presenter.removeEntitiesFromUI(m_selectedEntities);

        auto command =
                m_commandStack.addCommand<DeleteEntitiesCommand>(this, restoreInformation, uiDeletedInformation);
        if(!command) {
            RB_EDITOR_ERROR("EditorLogic: Can't Create DeleteCommand");
        }

        for(auto ent: m_selectedEntities) {
            m_activeScene -> removeEntity(ent);
            /// \brief send message to ScenePanel to remove from hiearchy
            auto* msg = m_messageBus.postMessage<EntityRemovement>(EntityRemove);
            msg -> entityID = ent.getComponent<IDComponent>().ID;
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

    std::vector<robot2D::ecs::Entity> EditorLogic::getEntities() const {
        return m_activeScene -> getEntities();
    }

    void EditorLogic::removeEntity(robot2D::ecs::Entity entity) {
        m_activeScene -> removeEntity(entity);
    }

    void EditorLogic::addEmptyEntity() {
        m_activeScene -> addEmptyEntity();
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
        return false;
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
            for(auto& child: tx.getChildren()) {
                if(child.getIndex() == graphicsEntityID) {
                    m_selectedEntities.emplace_back(child);
                    auto* msg =
                            m_messageBus.postMessage<PanelEntitySelectedMessage>(MessageID::PanelEntityNeedSelect);
                    msg -> entity = child;
                    return child;
                }
            }
        }

        if(m_selectedEntities.empty())
            m_presenter.clearSelectionOnUI();

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

    void EditorLogic::destroy() {
        TaskQueue::GetQueue() -> stop();
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

    //////////////////////////////////////// UIInteractor ////////////////////////////////////////

} // namespace editor



