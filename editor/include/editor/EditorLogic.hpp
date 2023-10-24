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

#pragma once
#include <vector>

#include "Project.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"
#include "MessageDispather.hpp"
#include "Messages.hpp"
#include "TaskQueue.hpp"
#include "Observer.hpp"
#include "PopupConfiguration.hpp"
#include "PopupManager.hpp"
#include "CommandStack.hpp"

#include "ScriptInteractor.hpp"
#include "EditorInteractor.hpp"
#include "EditorPresenter.hpp"
#include "EditorRouter.hpp"

namespace editor {

    class IEditor;


    class EditorLogic: public EditorInteractor, public ScriptInteractor, public PopupObserver {
    public:
        EditorLogic(robot2D::MessageBus& messageBus,
                    MessageDispatcher& messageDispatcher,
                    EditorPresenter& presenter,
                    EditorRouter& router
                    );

        EditorLogic(const EditorLogic& other) = delete;
        EditorLogic& operator=(const EditorLogic& other) = delete;
        EditorLogic(EditorLogic&& other) = delete;
        EditorLogic& operator=(EditorLogic&& other) = delete;
        ~EditorLogic() override = default;

        void setView(IEditor* iEditor) {
            PopupManager::getManager() -> addObserver(this);
        }

        //////////////////////////////////////// EditorInteractor ////////////////////////////////////////

        void update(float dt) override;
        void findSelectEntities(const robot2D::FloatRect& rect) override;
        bool hasSelectedEntities() const override;
        void createScene() override;
        void copyToBuffer() override;
        void pasterFromBuffer() override;
        void deleteEntity() override;
        void undoCommand() override;
        void redoCommand() override;
        void duplicateEntity(robot2D::vec2f mousePos, robot2D::ecs::Entity entity) override;
        void removeSelectedEntities() override;
        /// TODO(a.raag): Rename to PopupObservers for correct understanding
        void addObserver(Observer::Ptr observer) override;
        void notifyObservers(std::vector<std::string>&& paths) override;
        EditorState getState() const override;
        bool saveScene() override;

        //////////////////////////////////////// EditorInteractor ////////////////////////////////////////


        void createProject(Project::Ptr project);
        void loadProject(Project::Ptr project);

        //////////////////////////////////////// UIInteractor ////////////////////////////////////////
        robot2D::ecs::Entity getSelectedEntity(int graphicsEntityID)  override;
        std::vector<robot2D::ecs::Entity>& getSelectedEntities()  override;
        std::string getAssociatedProjectPath() const override;
        std::vector<robot2D::ecs::Entity> getEntities() const override;
        void removeEntity(robot2D::ecs::Entity entity) override;
        void addEmptyEntity() override;
        robot2D::ecs::Entity createEmptyEntity() override;
        robot2D::ecs::Entity duplicateEmptyEntity(robot2D::ecs::Entity entity) override;
        void setBefore(robot2D::ecs::Entity sourceEntity, robot2D::ecs::Entity target) override;
        void removeEntityChild(robot2D::ecs::Entity entity) override;
        bool isRunning() const override;
        robot2D::ecs::Entity getByUUID(UUID uuid) override;
        void registerOnDeleteFinish(std::function<void()>&& callback) override;
        void restoreDeletedEntities(DeletedEntitiesRestoreInformation& restoreInformation,
                                    DeletedEntitiesRestoreUIInformation& restoreUiInformation) override;
        //////////////////////////////////////// UIInteractor ////////////////////////////////////////

        //////////////////// ScriptInteractor ////////////////////
        robot2D::ecs::Entity getByUUID(std::uint64_t uuid) override;
        //////////////////// ScriptInteractor ////////////////////
    private:
        void loadCallback();
        void loadAssetsByEntity(robot2D::ecs::Entity entity);
        void saveScene(const MenuProjectMessage& message);
        void toolbarPressed(const ToolbarMessage& message);
        void openScene(const OpenSceneMessage& message);
        void createScene(const CreateSceneMessage& message);
        void addPrefabEntity(const PrefabLoadMessage& message);

        void onBeginPopup() override;
        void onEndPopup() override;
    private:
        robot2D::MessageBus& m_messageBus;
        MessageDispatcher& m_messageDispatcher;
        EditorPresenter& m_presenter;
        EditorRouter& m_router;
        CommandStack m_commandStack;

        Project::Ptr m_currentProject;
        SceneManager m_sceneManager;
        Scene::Ptr m_activeScene;

        std::vector<Observer::Ptr> m_observers;
        PopupConfiguration m_popupConfiguration;

        std::vector<robot2D::ecs::Entity> m_selectedEntities;
        std::vector<robot2D::ecs::Entity> m_copyEntities;
    };
}