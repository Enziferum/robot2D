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
// textCalculation options
#include <imgui/imgui_internal.h>
#include <tfd/tinyfiledialogs.h>
#include <editor/ProjectCreator.hpp>

namespace editor {

    const ImVec2 createButtonSize = ImVec2(200.F, 50.F);

    ProjectCreator::ProjectCreator(robot2D::RenderWindow& window): m_window{window}, m_descriptions() {}

    void ProjectCreator::setProjects(std::vector<ProjectDescription>&& desciptions) {
        m_descriptions = std::move(desciptions);
    }

    void ProjectCreator::setup(ProcessFunction&& createFunction, ProcessFunction&& deleteFunction,
                               ProcessFunction&& loadFunction) {
        m_createFunction = std::move(createFunction);
        m_deleteFunction = std::move(deleteFunction);
        m_loadFunction = std::move(loadFunction);
    }

    void ProjectCreator::render() {
        static bool isOpen = true;
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
        auto windowSize = m_window.getSize();

        ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y));
        ImGui::Begin("ProjectCreator", &isOpen, windowFlags);
        ImGui::SetWindowPos(ImVec2(0.F, 0.F));

        static const float textOffset = 10.F;
        static const float fontSize = ImGui::GetFontSize();

        int m_selectedItem = -1;
        auto halfWidth = windowSize.x / 2;
        auto font = ImGui::GetFont();
        auto height = ImGui::GetContentRegionAvail().y;
        ImGuiSelectableFlags_ selectableFlags = ImGuiSelectableFlags_AllowItemOverlap;

        if (ImGui::BeginListBox("##label", ImVec2(halfWidth, height)))
        {
            ImVec2 startPos{textOffset, textOffset};

            for (int it = 0; it < m_descriptions.size(); ++it)
            {
                auto& description = m_descriptions[it];
                std::string itemid = "##" + std::to_string(it);

                ImGui::PushID(it);

                ImVec2 nameTextSize = font -> CalcTextSizeA(fontSize, halfWidth,
                                                        halfWidth, description.name.c_str());
                ImVec2 pathTextSize = font -> CalcTextSizeA(fontSize, halfWidth,
                                                            halfWidth, description.path.c_str());
                ImVec2 textSize = ImVec2(nameTextSize.x + pathTextSize.x + 2 * textOffset,
                                         nameTextSize.y + pathTextSize.y + 2 * textOffset);

                if (ImGui::Selectable(itemid.c_str(), it == m_selectedItem, selectableFlags,
                                      ImVec2(textSize.x, textSize.y))) {
                    m_selectedItem = it;
                    ImGui::OpenPopup(itemid.c_str());
                }



                if(ImGui::BeginPopup(itemid.c_str())) {
                    if(ImGui::MenuItem("Load")) {
                        loadProject(it);
                        ImGui::CloseCurrentPopup();
                    }
                    if(ImGui::MenuItem("Delete")) {
                        deleteProject(it);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                auto drawList = ImGui::GetWindowDrawList();
                drawList -> AddText(font, fontSize, startPos, 0xFFFFFFFF,
                                    description.name.c_str(), nullptr, halfWidth);
                startPos.y += nameTextSize.y;
                drawList -> AddText(font, fontSize, ImVec2(startPos.x, startPos.y + textOffset), 0xFFFFFFFF,
                                    description.path.c_str(), nullptr, halfWidth);

                startPos.y += pathTextSize.y + 3 * textOffset;

                ImGui::Separator();
                ImGui::PopID();
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
        auto project = m_descriptions[index];
        m_descriptions.erase(m_descriptions.begin() + index);
        m_deleteFunction(project);
    }

    void ProjectCreator::createProject() {
        char* path = tinyfd_selectFolderDialog("Create Robot2D Project", nullptr);
        if(!path)
            return;
        std::string creationPath(path);
        m_createFunction({"Project", creationPath});
    }

    void ProjectCreator::loadProject(const unsigned int& index) {
        auto project = m_descriptions[index];
        m_loadFunction(project);
    }

}

