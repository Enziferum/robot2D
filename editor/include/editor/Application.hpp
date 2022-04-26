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

#include <memory>
#include <vector>

#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Core/MessageBus.hpp>
#include <robot2D/Extra/Gui.hpp>

#include <robot2D/Application.hpp>

#include "Editor.hpp"
#include "ProjectInspector.hpp"
#include "ProjectManager.hpp"
#include "EditorCache.hpp"
#include "Configuration.hpp"
#include "MessageQueue.hpp"

namespace editor {

    struct ApplicationConfiguration {
        const std::string logoPath = "res/textures/logo.png";
        const robot2D::vec2u inspectorSize = {600, 400};
    };

    class Application: public robot2D::Application {
    public:
        Application();
        ~Application() = default;

        void setup() override;
        void handleEvents(const robot2D::Event& event) override;
        void handleMessages();
        void update(float dt) override;
        void guiUpdate(float dt)override;
        void render() override;
    private:
        /// Subsription Api callbacks ///
        void createProject(ProjectDescription&& project);
        void loadProject(ProjectDescription&& project);
        void deleteProject(ProjectDescription&& projectDescription);
        /// Subsription Api callbacks ///
    private:
        enum class State {
            ProjectInspector,
            Editor
        };
    private:
        ApplicationConfiguration m_appConfiguration;

        robot2D::MessageBus m_messageBus;
        ImGui::Gui m_guiWrapper;
        Editor m_editor;
        State m_state;

        Configuration m_configuration;
        EditorCache m_editorCache;
        ProjectManager m_projectManager;
        ProjectInspector m_projectInspector;
    };
}

