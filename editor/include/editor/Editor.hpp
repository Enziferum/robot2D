/*********************************************************************
(c) Alex Raag 2021
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
#include <robot2D/Graphics/FrameBuffer.hpp>
#include <robot2D/Util/ResourceHandler.hpp>
#include <robot2D/imgui/Gui.hpp>

#include "UIManager.hpp"
#include "Scene.hpp"
#include "SceneManager.hpp"
#include "Project.hpp"
#include "MessageDispather.hpp"
#include "TaskQueue.hpp"
#include "EditorCamera.hpp"
#include "EditorLogic.hpp"
#include "EventBinder.hpp"
#include "Guizmo2D.hpp"

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
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
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
        virtual void prepare() = 0;
    };

    class Editor: public IEditor {
    public:
        Editor(robot2D::MessageBus& messageBus, robot2D::Gui& gui);
        Editor(const Editor&)=delete;
        Editor(const Editor&&)=delete;
        Editor& operator=(const Editor&)=delete;
        Editor& operator=(const Editor&&)=delete;
        ~Editor() = default;

        void setup(robot2D::RenderWindow* window, EditorLogic* editorLogic);
        void handleEvents(const robot2D::Event& event);
        void handleMessages(const robot2D::Message& message);
        void update(float dt);
        void render();



        void openScene(Scene::Ptr scene, std::string path) override;
        void prepare() override;
    private:
        void guiRender();
        void windowFunction();
        bool createScene();

        void setupBindings();
        void onSceneRun();
        void onSceneEdit();

        void drawScene();

        void onKeyPressed(const robot2D::Event& event);
        void onKeyReleased(const robot2D::Event& event);
    private:
        enum class State {
            Load,
            Edit,
            Run
        };
    private:
        State m_state;

        robot2D::RenderWindow* m_window;
        robot2D::MessageBus& m_messageBus;
        MessageDispatcher m_messageDispather;

        robot2D::Gui& m_gui;

        UIManager m_panelManager;
        Scene::Ptr m_activeScene{nullptr};
        EditorLogic* m_logic{nullptr};
        EventBinder m_eventBinder;

        EditorConfiguration m_configuration;

        robot2D::FrameBuffer::Ptr m_frameBuffer;
        robot2D::ResourceHandler<robot2D::Texture,
                        EditorConfiguration::TextureID> m_textures;
        robot2D::Color m_sceneClearColor;
        IEditorCamera::Ptr m_editorCamera;
        bool m_needPrepare{true};
        Guizmo2D m_guizmo2D;
    };
}