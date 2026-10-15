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
#include <imgui/imgui.h>
#include <robot2D/Core/MessageBus.hpp>

#include "ProjectDescription.hpp"

namespace editor {

    struct ProjectInspectorConfiguration {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking |
                                       ImGuiWindowFlags_NoSavedSettings;
        int selectedItem = -1;
        bool openAlways = true;
    };

    class ProjectInspector final {
    public:
        enum class CallbackType {
            Create, Load, Delete
        };
    public:
        ProjectInspector(robot2D::MessageBus& messageBus);
        ProjectInspector(const ProjectInspector& other) = delete;
        ProjectInspector& operator=(const ProjectInspector& other) = delete;
        ProjectInspector(ProjectInspector&& other) = delete;
        ProjectInspector& operator=(ProjectInspector&& other) = delete;
        ~ProjectInspector() = default;

        void setup(const std::vector<ProjectDescription>& descriptions, bool openAlways);
        void render();
    private:
        void renderProjects(float height);
        void renderActions(bool compact);
        void renderDeleteConfirmation();
        void createProject();
        void addProject();

        void loadProject(const unsigned& index);
        void removeProject(const unsigned& index, bool deleteFiles);
        bool validateProject(const ProjectDescription& description);
    private:
        robot2D::MessageBus& m_messageBus;
        std::vector<ProjectDescription> m_descriptions;
        ProjectInspectorConfiguration m_configuration;

        ImGuiTextFilter m_filter;
        int m_pendingDelete = -1;
        std::string m_error;
    };
} // namespace editor
