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

#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Core/MessageBus.hpp>
#include <robot2D/Graphics/FrameBuffer.hpp>
#include <robot2D/Util/ResourceHandler.hpp>

#include "PanelManager.hpp"
#include "Scene.hpp"
#include "SceneCamera.hpp"
#include "Project.hpp"

namespace editor {

    enum class TextureID {
        Face, Logo, White
    };

    class Editor {
    public:
        Editor(robot2D::RenderWindow& window, robot2D::MessageBus& messageBus);
        Editor(const Editor&)=delete;
        Editor(const Editor&&)=delete;
        Editor& operator=(const Editor&)=delete;
        Editor& operator=(const Editor&&)=delete;
        ~Editor() = default;

        void setup();
        void handleEvents(const robot2D::Event& event);
        void handleMessages(const robot2D::Message& message);
        void update(float dt);
        void render();

        void createProject(Project::Ptr project);
        void loadProject(Project::Ptr project);
    private:
        void prepare();

        void guiRender();
        void mainMenubar();

        bool createScene();
        bool openScene(const std::string& path);
        bool saveScene(const std::string& path);
    private:
        enum class State {
            Edit,
            Run
        };
    private:
        State m_state;

        robot2D::RenderWindow& m_window;
        robot2D::MessageBus& m_messageBus;
        PanelManager m_panelManager;
        Project::Ptr m_currentProject;
        Scene::Ptr m_activeScene;
        SceneCamera m_camera;

        robot2D::FrameBuffer::Ptr m_frameBuffer;
        robot2D::ResourceHandler<robot2D::Texture, TextureID> m_textures;
        robot2D::vec2u m_ViewportSize;
        robot2D::Color m_sceneClearColor;
        float m_dt = 0.F;
    };
}