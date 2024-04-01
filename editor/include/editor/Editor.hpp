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

#include <robot2D/Core/MessageBus.hpp>
#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Graphics/Sprite.hpp>
#include <robot2D/Graphics/FrameBuffer.hpp>
#include <robot2D/Util/ResourceHandler.hpp>
#include <robot2D/imgui/Gui.hpp>

#include "UIManager.hpp"
#include "SceneManager.hpp"
#include "ShortCutManager.hpp"
#include "PrefabManager.hpp"
#include "PopupManager.hpp"

#include "Scene.hpp"
#include "Project.hpp"
#include "MessageDispather.hpp"
#include "TaskQueue.hpp"
#include "EditorCamera.hpp"
#include "EditorLogic.hpp"
#include "EditorInteractor.hpp"
#include "ObjectManipulator.hpp"

#include "EventBinder.hpp"
#include "Guizmo2D.hpp"
#include "SceneGrid.hpp"
#include "CameraManipulator.hpp"
#include "PopupConfiguration.hpp"
#include "SceneRender.hpp"
#include "SelectionCollider.hpp"
#include "DeletedEntitesRestoreInformation.hpp"

namespace editor {

    struct EditorConfiguration {
        enum class TextureID {
            Face, Logo, White, RunButton, EditButton
        };

        const bool useGUI = true;
        const std::string texturesPath = "res/textures/";
        bool m_leftCtrlPressed = false;
        bool opt_padding = true;
        bool dockspace_open = false;
        std::unordered_map<TextureID, std::string> texturePaths = {
                {TextureID::Logo, "logo.png"},
                {TextureID::EditButton, "StopButton.png"},
                {TextureID::RunButton, "PlayButton.png"},
        };
    };


    class IEditor {
    public:
        virtual ~IEditor() = 0;
        virtual void openScene(Scene::Ptr scene, std::string path) = 0;
        virtual void prepareView() = 0;

        virtual void clearSelectionOnUI() = 0;
        virtual DeletedEntitiesRestoreUIInformation removeEntitiesOnUI(std::vector<ITreeItem::Ptr>&& uiItems) = 0;
        virtual void restoreEntitiesOnUI(DeletedEntitiesRestoreUIInformation& restoreUiInformation) = 0;

        virtual void findSelectedEntitiesOnUI(std::vector<ITreeItem::Ptr>&& item) = 0;
        virtual void showPopup(PopupConfiguration* configuration) = 0;
        virtual void setMainCameraEntity(robot2D::ecs::Entity entity) = 0;
    };

    class Editor: public IEditor, public PopupDelegate {
    public:
        Editor(robot2D::MessageBus& messageBus, robot2D::Gui& gui);
        Editor(const Editor&)=delete;
        Editor(const Editor&&)=delete;
        Editor& operator=(const Editor&)=delete;
        Editor& operator=(const Editor&&)=delete;
        ~Editor() override = default;

        void setup(robot2D::RenderWindow* window, EditorInteractor* editorInteractor);
        void handleEvents(const robot2D::Event& event);
        void handleMessages(const robot2D::Message& message);
        void update(float dt);
        void render();

        /////////////////////////// IEditor ///////////////////////////
        void openScene(Scene::Ptr scene, std::string path) override;
        void prepareView() override;
        void showPopup(editor::PopupConfiguration* configuration) override;
        void findSelectedEntitiesOnUI(std::vector<ITreeItem::Ptr>&& items) override;
        void clearSelectionOnUI() override;
        DeletedEntitiesRestoreUIInformation removeEntitiesOnUI(std::vector<ITreeItem::Ptr>&& uiItems) override;
        void restoreEntitiesOnUI(DeletedEntitiesRestoreUIInformation& restoreUiInformation) override;
        virtual void setMainCameraEntity(robot2D::ecs::Entity entity) override;
        /////////////////////////// IEditor ///////////////////////////

    private:
        void guiRender();
        void setupBindings();
        void setupShortCuts();
        void setupSpinner();

        void onKeyPressed(const robot2D::Event& event);
        void onKeyReleased(const robot2D::Event& event);
        void onMousePressed (const robot2D::Event& event);
        void onMouseReleased(const robot2D::Event& event);
        void onMouseMoved(const robot2D::Event& event);

        void onPopupRender() override;
    private:
        enum class State {
            LostFocus,
            HasFocus
        } m_state = State::HasFocus;
    private:
        enum class EditorShortCutType {
            GizmoMove,
            GizmoScale,
            SaveScene,
            Copy,
            Duplicate,
            Paste,
            Undo
        };

        robot2D::RenderWindow* m_window;
        robot2D::MessageBus& m_messageBus;
        MessageDispatcher m_messageDispather;
        EditorInteractor* m_interactor{nullptr};

        UIManager m_panelManager;
        EventBinder m_eventBinder;

        robot2D::FrameBuffer::Ptr m_frameBuffer;
        robot2D::FrameBuffer::Ptr m_gameFrameBuffer;
        Scene::Ptr m_activeScene{nullptr};
        IEditorCamera::Ptr m_editorCamera;

        EditorConfiguration m_configuration;

        robot2D::ResourceHandler<robot2D::Texture,
                        EditorConfiguration::TextureID> m_textures;

        ShortCutManager<EditorShortCutType> m_shortcutManager;
        SceneGrid m_sceneGrid;

        Guizmo2D m_guizmo2D;
        CameraManipulator m_cameraManipulator;
        SelectionCollider m_selectionCollider;
        ObjectManipulator m_objectManipulator;

        bool m_leftMousePressed{false};
        bool m_needPrepareView{true};

        editor::PopupConfiguration* m_popupConfiguration{nullptr};
        SceneRender m_sceneRender;
        /// TODO(a.raag) move to Editor Logic ?
        PrefabManager m_prefabManager;
    };
}