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
#include <imgui/imgui_internal.h>
#include <tfd/tinyfiledialogs.h>

#include <editor/ProjectInspector.hpp>
#include <editor/Messages.hpp>

namespace editor {

    ProjectInspector::ProjectInspector(robot2D::MessageBus& messageBus):
    m_window{nullptr},
    m_messageBus{messageBus},
    m_descriptions(),
    m_configuration{}{}

    void ProjectInspector::setup(robot2D::RenderWindow* window,
                                 const std::vector<ProjectDescription>& descriptions) {
        if(m_window == nullptr)
            m_window = window;
        m_descriptions = descriptions;
    }

    void ProjectInspector::render() {
        auto font = ImGui::GetFont();
        auto windowSize = m_window -> getSize();

        ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y));
        ImGui::Begin("ProjectInspector", &m_configuration.isOpen, m_configuration.windowFlags);
        ImGui::SetWindowPos(ImVec2(0.F, 0.F));

        static const float fontSize = ImGui::GetFontSize();

        auto halfWidth = windowSize.x / 2;
        auto height = ImGui::GetContentRegionAvail().y;

        if (ImGui::BeginListBox("##label", ImVec2(halfWidth, height)))
        {
            ImVec2 startPos{m_configuration.textOffset, m_configuration.textOffset};

            for (int it = 0; it < m_descriptions.size(); ++it)
            {
                auto& description = m_descriptions[it];
                std::string itemid = "##" + std::to_string(it);

                ImGui::PushID(it);

                ImVec2 nameTextSize = font -> CalcTextSizeA(fontSize, halfWidth,
                                                        halfWidth, description.name.c_str());
                ImVec2 pathTextSize = font -> CalcTextSizeA(fontSize, halfWidth,
                                                            halfWidth, description.path.c_str());
                ImVec2 textSize = ImVec2(nameTextSize.x + pathTextSize.x + 2 * m_configuration.textOffset,
                                         nameTextSize.y + pathTextSize.y + 2 * m_configuration.textOffset);

                if (ImGui::Selectable(itemid.c_str(), it == m_configuration.m_selectedItem,
                                      m_configuration.selectableFlags,
                                      ImVec2(textSize.x, textSize.y))) {
                    m_configuration.m_selectedItem = it;
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
                drawList -> AddText(font, fontSize, startPos, m_configuration.colID,
                                    description.name.c_str(), nullptr, halfWidth);
                startPos.y += nameTextSize.y;
                drawList -> AddText(font, fontSize, ImVec2(startPos.x, startPos.y + m_configuration.textOffset),
                                    m_configuration.colID,
                                    description.path.c_str(), nullptr, halfWidth);

                startPos.y += pathTextSize.y + 3 * m_configuration.textOffset;

                ImGui::Separator();
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }

        ImGui::SameLine();
        ImGui::BeginGroup();

        if(ImGui::Button("Create Project", m_configuration.createButtonSize))
            createProject();
        if(ImGui::Checkbox("Open always", &m_configuration.openAlways)) {
            auto* msg = m_messageBus.postMessage<ShowInspectorMessage>(MessageID::ShowInspector);
            msg -> showAlways = m_configuration.openAlways;
        }

        ImGui::EndGroup();

        ImGui::End();
    }


    void ProjectInspector::createProject() {
        char* path = tinyfd_selectFolderDialog("Create Robot2D Project", nullptr);
        // close, either
        if(path == nullptr) {
            return;
        }
        std::string creationPath(path);
        ProjectDescription description;
        description.name = "Project";
        description.path = creationPath;

        auto* msg = m_messageBus.postMessage<ProjectMessage>(MessageID::CreateProject);
        msg -> description = std::move(description);
    }

    void ProjectInspector::loadProject(const unsigned int& index) {
        auto* msg = m_messageBus.postMessage<ProjectMessage>(MessageID::LoadProject);
        msg -> description = m_descriptions[index];
    }

    void ProjectInspector::deleteProject(const unsigned int& index) {
        assert(index < m_descriptions.size() && "Index out of Range");
        auto project = m_descriptions[index];
        m_descriptions.erase(m_descriptions.begin() + index);
        auto* msg = m_messageBus.postMessage<ProjectMessage>(MessageID::DeleteProject);
        msg -> description = project;
    }

}

