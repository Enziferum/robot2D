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

#include <cassert>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <robot2D/imgui/Sugar.hpp>
#include <robot2D/Config.hpp>
#include <editor/ProjectInspector.hpp>
#include <editor/Messages.hpp>
#include <editor/FiledialogAdapter.hpp>
#include <editor/Buffer.hpp>


namespace editor {

    std::vector<std::string> split(const std::string& str, std::string separator) {
        std::vector<std::string> res;
        const auto sepSize = separator.size();
        std::string::size_type start{ 0 }, end;

        while ((end = str.find(separator, start)) != std::string::npos) {
            res.emplace_back(str.substr(start, end - start));
            start = end + sepSize;
        }

        res.emplace_back(str.substr(start));

        return res;
    }



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

        imgui_ListBox("##label", ImVec2(halfWidth, height)) {
            ImVec2 startPos{ m_configuration.textOffset, m_configuration.textOffset };

            for (int it = 0; it < m_descriptions.size(); ++it)
            {
                auto& description = m_descriptions[it];
                std::string itemid = "##" + std::to_string(it);

                ImGui::PushID(it);

                ImVec2 nameTextSize = font->CalcTextSizeA(fontSize, halfWidth, halfWidth, description.name.c_str());
                ImVec2 pathTextSize = font->CalcTextSizeA(fontSize, halfWidth, halfWidth, description.path.c_str());
                ImVec2 textSize = ImVec2(nameTextSize.x + pathTextSize.x + 2 * m_configuration.textOffset,
                    nameTextSize.y + pathTextSize.y + 2 * m_configuration.textOffset);

                if (ImGui::Selectable(itemid.c_str(), it == m_configuration.m_selectedItem,
                    m_configuration.selectableFlags,
                    ImVec2(textSize.x, textSize.y))) {
                    m_configuration.m_selectedItem = it;
                    ImGui::OpenPopup(itemid.c_str());
                }

                if (ImGui::BeginPopup(itemid.c_str())) {
                    if (ImGui::MenuItem("Load")) {
                        loadProject(it);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Remove from list")) {
                        // TODO(a.raag): remove from cache
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::MenuItem("Delete")) {
                        deleteProject(it);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                auto drawList = ImGui::GetWindowDrawList();

                drawList->AddText(font, fontSize, startPos, m_configuration.colID,
                    description.name.c_str(), nullptr, halfWidth);
                startPos.y += nameTextSize.y;
                drawList->AddText(font, fontSize, ImVec2(startPos.x, startPos.y + m_configuration.textOffset),
                    m_configuration.colID,
                    description.path.c_str(), nullptr, halfWidth);

                startPos.y += pathTextSize.y + 3 * m_configuration.textOffset;

                ImGui::Separator();
                ImGui::PopID();
            }
        }


        ImGui::SameLine();
        ImGui::BeginGroup();

        if(ImGui::Button("Create Project", m_configuration.createButtonSize))
            createProject();
        if(ImGui::Button("Add Existing", m_configuration.createButtonSize))
            addProject();


        if(ImGui::Checkbox("Open always", &m_configuration.openAlways)) {
            auto* msg = m_messageBus.postMessage<ShowInspectorMessage>(MessageID::ShowInspector);
            msg -> showAlways = m_configuration.openAlways;
        }

        ImGui::EndGroup();

        ImGui::End();
    }

    void ProjectInspector::createProject() {

        std::string creationPath;
        if(!FiledialogAdapter::get() -> selectFolder(creationPath, "Create Robot2D Project")) {
            return;
        }

        std::string separator;
#ifdef ROBOT2D_WINDOWS
        separator = "\\";
#else
        separator = "/";
#endif
        auto pathComps = split(creationPath, separator);

        m_projectName = pathComps[pathComps.size() - 1];
        m_projectPath = std::move(creationPath);

      
        int allocSize = StringBuffer::calcAllocSize(m_projectName) + StringBuffer::calcAllocSize(m_projectPath);
        void* rawBuffer = m_messageBus.postMessage(MessageID::CreateProject, allocSize);

        Buffer buffer{ rawBuffer };
        pack_message_string(m_projectName, buffer);
        pack_message_string(m_projectPath, buffer);
    }

    void ProjectInspector::addProject() {
        std::string addPath;
        if(!FiledialogAdapter::get() -> selectFolder(addPath, "Add Existing Robot2D Project")) {
            return;
        }
        std::string separator;
#ifdef ROBOT2D_WINDOWS
        separator = "\\";
#else
        separator = "/";
#endif
        auto pathComps = split(addPath, separator);

        m_projectName = pathComps[pathComps.size() - 1];
        m_projectPath = std::move(addPath);

  
        int allocSize = StringBuffer::calcAllocSize(m_projectName) + StringBuffer::calcAllocSize(m_projectPath);
        void* rawBuffer = m_messageBus.postMessage(MessageID::AddProject, allocSize);

        Buffer buffer{ rawBuffer };
        pack_message_string(m_projectName, buffer);
        pack_message_string(m_projectPath, buffer);
    }

    void ProjectInspector::loadProject(const unsigned int& index) {
        auto& desc = m_descriptions[index];
        int allocSize = StringBuffer::calcAllocSize(desc.name) + StringBuffer::calcAllocSize(desc.path);
        void* rawBuffer = m_messageBus.postMessage(MessageID::LoadProject, allocSize);
        
        Buffer buffer{ rawBuffer };
        pack_message_string(desc.name, buffer);
        pack_message_string(desc.path, buffer);
    }

    void ProjectInspector::deleteProject(const unsigned int& index) {
        assert(index < m_descriptions.size() && "Index out of Range");
        auto project = m_descriptions[index];
        m_descriptions.erase(m_descriptions.begin() + index);
        int allocSize = StringBuffer::calcAllocSize(project.name) + StringBuffer::calcAllocSize(project.path);
        void* rawBuffer = m_messageBus.postMessage(MessageID::DeleteProject, allocSize);
        
        Buffer buffer{ rawBuffer };
        pack_message_string(project.name, buffer);
        pack_message_string(project.path, buffer);
    }
}

