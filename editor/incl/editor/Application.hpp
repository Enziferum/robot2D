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
#include "Wrapper.hpp"
#include "Editor.hpp"
#include "ProjectCreator.hpp"
#include "Project.hpp"

namespace editor {
    class Application {
    public:
        Application();
        ~Application() = default;

        void run();
    private:
        void setup();
        void handleEvents();
        void handleMessages();
        void update(float dt);
        void render();

        void deleteProject(std::string path);
        void createProject(ProjectDescription project);
        void loadProject(ProjectDescription project);

        bool parseProjectCache();
    private:
        robot2D::vec2u defaultWindowSize;
        robot2D::RenderWindow m_window;
        robot2D::MessageBus m_messageBus;
        ImGui::Wrapper m_guiWrapper;
        Editor m_editor;

        enum class State {
            CreateProject,
            Editor
        };

        State m_state;
        ProjectCreator m_projectCreator;
        std::vector<Project::Ptr> m_projectsCache;
    };
}

