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
#include <editor/scripting/ScriptingEngineService.hpp>

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
#include "QuadTree.hpp"

namespace editor {

    class EditorLogic: public EditorInteractor, public PopupObserver {
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

        void setup(IScriptInteractorFrom::WeakPtr scriptInteractor);
        void destroy();

        void createProject(Project::Ptr project);
        void loadProject(Project::Ptr project);



        //////////////////////////////////////// EditorInteractor ////////////////////////////////////////
        void handleEventsRuntime(const robot2D::Event& event) override;
        void update(float dt) override;
        bool hasSelectedEntities() const override;
        void findSelectEntities(const robot2D::FloatRect& rect) override;
        void copyToBuffer() override;
        void pasterFromBuffer() override;
        void undoCommand() override;
        void redoCommand() override;
        void duplicateEntity(robot2D::vec2f mousePos) override;
        void removeSelectedEntities() override;
        void removeEntities(std::vector<SceneEntity>& entities, bool clearContainer) override;


        EditorState getState() const override;
        /// \brief Save Scene by Shortcut.
        bool saveScene() override;

        void closeCurrentProject(std::function<void()>&& resultCallback);
        robot2D::vec2f getMainCameraPosition() const override;
        void setMainCamera(SceneEntity cameraEntity) override;
        void setEditorCamera(IEditorCamera::Ptr editorCamera) override;
        //////////////////////////////////////// EditorInteractor ////////////////////////////////////////


        //////////////////////////////////////// UIInteractor ////////////////////////////////////////
        SceneEntity findEntity(const robot2D::vec2i& mousePos)  override;
        std::vector<SceneEntity>& getSelectedEntities()  override;
        std::string getAssociatedProjectPath() const override;
        const std::list<SceneEntity>& getEntities() const override;
        void addEmptyEntity() override;
        SceneEntity addButton() override;

        SceneEntity createEmptyEntity() override;
        /// \brief using when need create entity on scene, but don't want to notify someone
        SceneEntity duplicateEmptyEntity(SceneEntity entity) override;

        bool setBefore(SceneEntity sourceEntity, SceneEntity target) override;
        void removeEntityChild(SceneEntity entity) override;
        bool isRunning() const override;
        SceneEntity getEntity(UUID uuid) override;

        void restoreDeletedEntities(DeletedEntitiesRestoreInformation& restoreInformation,
                                    DeletedEntitiesRestoreUIInformation& restoreUiInformation) override;

        void uiSelectedEntities(std::set<ITreeItem::Ptr>& uiItems, bool isAll) override;

        const std::vector<class_id>& getCommandStack() const override;
        void exportProject(const ExportOptions& exportOptions) override;

        std::shared_ptr<IScriptInteractorFrom> getScriptInteractor() const override;
        //////////////////////////////////////// UIInteractor ////////////////////////////////////////
    private:
        void loadSceneCallback(Scene::Ptr loadedScene);
        void processEntity(SceneEntity entity);

        friend class EditorModule;
        void switchRuntimeState(const ToolbarMessage::Type& messageType);

        void saveScene(const MenuProjectMessage& message);

        void openScene(const OpenSceneMessage& message);
        void createScene(const CreateSceneMessage& message);
        void addPrefabEntity(const PrefabLoadMessage& message);
        void prefabModificated(const PrefabAssetModificatedMessage& message);
        void generateProject(const GenerateProjectMessage& message);

        void onBeginPopup() override;
        void onEndPopup() override;

        void pasteChild(SceneEntity parent);
    private:
        robot2D::MessageBus& m_messageBus;
        MessageDispatcher& m_messageDispatcher;
        EditorPresenter& m_presenter;
        EditorRouter& m_router;
        CommandStack m_commandStack;
        IScriptInteractorFrom::WeakPtr m_scriptInteractor;

        Project::Ptr m_currentProject;
        SceneManager m_sceneManager;
        Scene::Ptr m_activeScene;

        PopupConfiguration m_popupConfiguration;

        std::vector<SceneEntity> m_selectedEntities;
        std::vector<SceneEntity> m_copyEntities;

        SceneEntity m_mainCameraEntity;
        std::function<void()> m_closeResultProjectCallback{nullptr};

        QuadTreeContainer<SceneEntity> m_quadTree;
    };
}