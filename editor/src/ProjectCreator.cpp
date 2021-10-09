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

#include <cassert>
#include <imgui/imgui.h>
#include <tfd/tinyfiledialogs.h>
#include <editor/ProjectCreator.hpp>


namespace editor {

    const ImVec2 createButtonSize = ImVec2(200.F, 50.F);

    ProjectCreator::ProjectCreator(robot2D::RenderWindow& window): m_window{window}, m_descriptions() {}

    void ProjectCreator::setup(ProcessFunction&& createFunction, ProcessFunction&& deleteFunction) {
        m_createFunction = std::move(createFunction);
        m_deleteFunction = std::move(deleteFunction);

        m_descriptions = { {"Example Project", "Documents/dev/Example Project"},
                           {"UI", "Desktop/UI"}};
    }

    void ProjectCreator::render() {
        static bool isOpen = true;
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
        auto windowSize = m_window.getSize();

        ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y));
        ImGui::Begin("ProjectCreator", &isOpen, windowFlags);
        ImGui::SetWindowPos(ImVec2(0.F, 0.F));

        if (ImGui::BeginListBox("", ImVec2(windowSize.x / 2, windowSize.y)))
        {
            for (int it = 0; it < m_descriptions.size(); ++it)
            {
                auto& description = m_descriptions[it];

                ImGui::Text("%s", description.name.c_str());
                ImGui::Text("%s", description.path.c_str());
                ImGui::SameLine();
                if(ImGui::Button("Delete"))
                    deleteProject(it);
                ImGui::Separator();
            }
            ImGui::EndListBox();
        }
        ImGui::SameLine();
        if(ImGui::Button("Create Project", createButtonSize))
            createProject();

        ImGui::End();
    }

    void ProjectCreator::deleteProject(const unsigned int& index) {
        assert(index < m_descriptions.size() && "Index out of Range");
        auto path = m_descriptions[index].path;
        m_descriptions.erase(m_descriptions.begin() + index);
        m_deleteFunction(path);
    }

    void ProjectCreator::createProject() {
        char* path = tinyfd_selectFolderDialog("Create Robot2D Project", nullptr);
        if(!path)
            return;
        std::string creationPath(path);
        m_createFunction(creationPath);
    }

}

