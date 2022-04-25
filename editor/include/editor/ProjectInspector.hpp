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
#include <functional>
#include <vector>
#include <unordered_map>
#include <robot2D/Graphics/RenderWindow.hpp>

#include "ProjectDescription.hpp"
#include "EditorCache.hpp"

namespace editor {

    using ProcessFunction = std::function<void(ProjectDescription)>;

    struct ProjectInspectorConfiguration {
        const ImVec2 createButtonSize = ImVec2(200.F, 50.F);
        const float textOffset = 10.F;
        const unsigned colID = 0xFFFFFFFF;
        bool isOpen = true;
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
                                       | ImGuiWindowFlags_NoResize;
        ImGuiSelectableFlags_ selectableFlags = ImGuiSelectableFlags_AllowItemOverlap;
        int m_selectedItem = -1;
        bool openAlways = true;
    };

    class ProjectInspector final {
    public:
        enum class CallbackType {
            Create, Load, Delete
        };
    public:
        ProjectInspector(EditorCache& editorCache);
        ~ProjectInspector() = default;

        void addCallback(const CallbackType& callbackType, ProcessFunction&& function);
        void setup(robot2D::RenderWindow* window);
        void render();
    private:
        void createProject();
        void loadProject(const unsigned& index);
        void deleteProject(const unsigned& index);
    private:
        robot2D::RenderWindow* m_window;
        EditorCache& m_editorCache;
        std::vector<ProjectDescription> m_descriptions;

        std::unordered_map<CallbackType, ProcessFunction> m_functions;
        ProjectInspectorConfiguration m_configuration;
    };
}
