/*********************************************************************
(c) Alex Raag 2026
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

#include <algorithm>
#include <filesystem>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <editor/ProjectInspector.hpp>
#include <editor/Messages.hpp>
#include <editor/FiledialogAdapter.hpp>
#include <editor/Configuration.hpp>
#include <editor/Buffer.hpp>
#include <editor/BuildInfo.hpp>

namespace editor {

    namespace {
        bool postProjectMessage(robot2D::MessageBus& bus, robot2D::Message::ID type,
                                const ProjectDescription& description) {
            const auto size = StringBuffer::calcAllocSize(description.name) +
                              StringBuffer::calcAllocSize(description.path);
            auto* rawBuffer = bus.postMessage(type, size);
            if(!rawBuffer)
                return false;
            Buffer buffer{rawBuffer};
            pack_message_string(description.name, buffer);
            pack_message_string(description.path, buffer);
            return true;
        }

        ProjectDescription describeFolder(const std::string& folder) {
            auto path = std::filesystem::u8path(folder).lexically_normal();
            if(path.filename().empty())
                path = path.parent_path();
            ProjectDescription description;
            description.name = path.filename().u8string();
            description.path = path.u8string();
            return description;
        }
    }

    ProjectInspector::ProjectInspector(robot2D::MessageBus& messageBus):
    m_messageBus{messageBus},
    m_descriptions(),
    m_configuration{} {}

    void ProjectInspector::setup(const std::vector<ProjectDescription>& descriptions, bool openAlways) {
        m_descriptions = descriptions;
        m_configuration.openAlways = openAlways;
        m_configuration.selectedItem = -1;
        m_filter.Clear();
        m_pendingDelete = -1;
        m_error.clear();
    }

    void ProjectInspector::render() {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport -> WorkPos);
        ImGui::SetNextWindowSize(viewport -> WorkSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {18.f, 16.f});
        if(ImGui::Begin("ProjectInspector", nullptr, m_configuration.windowFlags)) {
            ImGui::SetWindowFontScale(1.25f);
            ImGui::TextUnformatted(("Robot2D " + buildInfo::getVersion()).c_str());
            ImGui::SetWindowFontScale(1.f);
            ImGui::TextDisabled("%s | Build %s", buildInfo::configuration, buildInfo::buildID);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Recent projects (%d)", static_cast<int>(m_descriptions.size()));
            ImGui::SetNextItemWidth(-1.f);
            if(ImGui::InputTextWithHint("##SearchProjects", "Search by name or path...",
                                       m_filter.InputBuf, IM_ARRAYSIZE(m_filter.InputBuf))) {
                m_filter.Build();
                m_configuration.selectedItem = -1;
                m_error.clear();
            }

            const bool compact = ImGui::GetContentRegionAvail().x < 460.f;
            const auto& style = ImGui::GetStyle();
            const float footerHeight = (compact ? 3.f : 2.f) *
                                       (ImGui::GetFrameHeight() + style.ItemSpacing.y);
            // Keep an error line reserved so failed opens don't move the list.
            const float errorHeight = ImGui::GetTextLineHeightWithSpacing();
            renderProjects(std::max(60.f, ImGui::GetContentRegionAvail().y - footerHeight -
                                          errorHeight - style.ItemSpacing.y));
            ImGui::PushStyleColor(ImGuiCol_Text, {0.95f, 0.58f, 0.44f, 1.f});
            if(m_error.empty())
                ImGui::Dummy({0.f, ImGui::GetTextLineHeight()});
            else {
                ImGui::TextUnformatted(m_error.c_str());
                if(ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 28.f);
                    ImGui::TextUnformatted(m_error.c_str());
                    ImGui::PopTextWrapPos();
                    ImGui::EndTooltip();
                }
            }
            ImGui::PopStyleColor();
            renderActions(compact);
            renderDeleteConfirmation();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ProjectInspector::renderProjects(float height) {
        int openIndex = -1;
        int removeIndex = -1;
        int deleteIndex = -1;
        if(ImGui::BeginChild("##RecentProjects", {0.f, height}, true,
                             ImGuiWindowFlags_NoSavedSettings)) {
            bool hasVisibleProjects = false;
            const auto lineHeight = ImGui::GetTextLineHeight();
            for(int i = 0; i < static_cast<int>(m_descriptions.size()); ++i) {
                const auto& description = m_descriptions[i];
                const auto searchText = description.name + "\n" + description.path;
                if(!m_filter.PassFilter(searchText.c_str()))
                    continue;
                hasVisibleProjects = true;
                ImGui::PushID(i);
                const auto position = ImGui::GetCursorScreenPos();
                const float width = ImGui::GetContentRegionAvail().x;
                const float rowHeight = 2.f * lineHeight + 14.f;
                if(ImGui::Selectable("##Project", i == m_configuration.selectedItem,
                                      ImGuiSelectableFlags_AllowDoubleClick, {width, rowHeight})) {
                    m_configuration.selectedItem = i;
                    m_error.clear();
                    if(ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        openIndex = i;
                }
                if(ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::PushTextWrapPos(std::min(ImGui::GetFontSize() * 32.f,
                                                   ImGui::GetMainViewport() -> WorkSize.x - 32.f));
                    ImGui::TextUnformatted(description.name.c_str());
                    ImGui::TextUnformatted(description.path.c_str());
                    ImGui::PopTextWrapPos();
                    ImGui::EndTooltip();
                }
                if(ImGui::BeginPopupContextItem("##ProjectActions")) {
                    m_configuration.selectedItem = i;
                    if(ImGui::MenuItem("Open"))
                        openIndex = i;
                    if(ImGui::MenuItem("Remove from recent projects"))
                        removeIndex = i;
                    ImGui::Separator();
                    if(ImGui::MenuItem("Delete project files..."))
                        deleteIndex = i;
                    ImGui::EndPopup();
                }
                auto* drawList = ImGui::GetWindowDrawList();
                const float right = position.x + width - 8.f;
                ImGui::RenderTextEllipsis(drawList, {position.x + 8.f, position.y + 5.f},
                                          {right, position.y + lineHeight + 5.f}, right, right,
                                          description.name.c_str(), description.name.c_str() + description.name.size(), nullptr);
                const auto pathColor = i == m_configuration.selectedItem
                    ? ImVec4{0.78f, 0.84f, 0.90f, 1.f} : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
                ImGui::PushStyleColor(ImGuiCol_Text, pathColor);
                ImGui::RenderTextEllipsis(drawList, {position.x + 8.f, position.y + lineHeight + 8.f},
                                          {right, position.y + 2.f * lineHeight + 8.f}, right, right,
                                          description.path.c_str(), description.path.c_str() + description.path.size(), nullptr);
                ImGui::PopStyleColor();
                ImGui::PopID();
            }
            if(!hasVisibleProjects) {
                ImGui::Spacing();
                ImGui::TextDisabled(m_descriptions.empty() ? "No recent projects yet." : "No matching projects.");
                if(m_descriptions.empty())
                    ImGui::TextWrapped("Create a project or add an existing project folder below.");
            }
        }
        ImGui::EndChild();
        // Mutate only after all rows and popups have finished using the vector.
        if(removeIndex >= 0)
            removeProject(removeIndex, false);
        else if(openIndex >= 0)
            loadProject(openIndex);
        if(deleteIndex >= 0) {
            m_pendingDelete = deleteIndex;
            ImGui::OpenPopup("Delete project?");
        }
    }

    void ProjectInspector::renderActions(bool compact) {
        const float width = ImGui::GetContentRegionAvail().x;
        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        const float buttonHeight = ImGui::GetFrameHeight();
        const float openWidth = compact ? width : (width - 2.f * spacing) / 3.f;
        ImGui::BeginDisabled(m_configuration.selectedItem < 0);
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Header));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        if(ImGui::Button("Open project", {openWidth, buttonHeight}))
            loadProject(m_configuration.selectedItem);
        ImGui::PopStyleColor(3);
        ImGui::EndDisabled();
        if(!compact)
            ImGui::SameLine();
        const float actionWidth = compact ? (width - spacing) / 2.f : openWidth;
        if(ImGui::Button("Create project", {actionWidth, buttonHeight}))
            createProject();
        ImGui::SameLine();
        if(ImGui::Button("Add existing", {actionWidth, buttonHeight}))
            addProject();
        if(ImGui::Checkbox("Show project browser on startup", &m_configuration.openAlways)) {
            auto* message = m_messageBus.postMessage<ShowInspectorMessage>(MessageID::ShowInspector);
            if(message)
                message -> showAlways = m_configuration.openAlways;
        }
    }

    void ProjectInspector::renderDeleteConfirmation() {
        ImGui::SetNextWindowSize({std::min(420.f, ImGui::GetMainViewport() -> WorkSize.x - 24.f), 0.f});
        if(ImGui::BeginPopupModal("Delete project?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            if(m_pendingDelete >= 0 && m_pendingDelete < static_cast<int>(m_descriptions.size())) {
                const auto& description = m_descriptions[m_pendingDelete];
                ImGui::TextWrapped("Permanently delete this project folder and all its files?");
                ImGui::Spacing();
                ImGui::TextWrapped("%s", description.name.c_str());
                ImGui::TextWrapped("%s", description.path.c_str());
                ImGui::Spacing();
                if(ImGui::Button("Delete files")) {
                    removeProject(m_pendingDelete, true);
                    m_pendingDelete = -1;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if(ImGui::Button("Cancel")) {
                    m_pendingDelete = -1;
                    ImGui::CloseCurrentPopup();
                }
            }
            else
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }

    bool ProjectInspector::validateProject(const ProjectDescription& description) {
        Configuration configuration;
        const auto [hasExtension, extension] = configuration.getValue(ConfigurationKey::ProjectExtension);
        std::error_code error;
        const auto file = std::filesystem::u8path(description.path) /
                          std::filesystem::u8path(description.name + extension);
        if(!hasExtension || description.name.empty() || description.path.empty() ||
           !std::filesystem::is_regular_file(file, error)) {
            m_error = "Project file not found. Check the folder or remove this entry.";
            return false;
        }
        return true;
    }

    void ProjectInspector::createProject() {
        std::string folder;
        if(!FiledialogAdapter::get() -> selectFolder(folder, "Create Robot2D Project"))
            return;
        const auto description = describeFolder(folder);
        std::error_code error;
        if(description.name.empty() || !std::filesystem::is_empty(std::filesystem::u8path(description.path), error) || error) {
            m_error = "Choose an empty folder for a new project.";
            return;
        }
        if(postProjectMessage(m_messageBus, MessageID::CreateProject, description))
            m_error.clear();
        else
            m_error = "Could not queue project creation.";
    }

    void ProjectInspector::addProject() {
        std::string folder;
        if(!FiledialogAdapter::get() -> selectFolder(folder, "Add Existing Robot2D Project"))
            return;
        const auto description = describeFolder(folder);
        if(!validateProject(description))
            return;
        for(int i = 0; i < static_cast<int>(m_descriptions.size()); ++i) {
            std::error_code error;
            if(std::filesystem::equivalent(std::filesystem::u8path(description.path),
                                           std::filesystem::u8path(m_descriptions[i].path), error)) {
                m_configuration.selectedItem = i;
                m_filter.Clear();
                loadProject(i);
                return;
            }
        }
        if(postProjectMessage(m_messageBus, MessageID::AddProject, description))
            m_error.clear();
        else
            m_error = "Could not queue project loading.";
    }

    void ProjectInspector::loadProject(const unsigned& index) {
        if(index >= m_descriptions.size() || !validateProject(m_descriptions[index]))
            return;
        if(postProjectMessage(m_messageBus, MessageID::LoadProject, m_descriptions[index]))
            m_error.clear();
        else
            m_error = "Could not queue project loading.";
    }

    void ProjectInspector::removeProject(const unsigned& index, bool deleteFiles) {
        if(index >= m_descriptions.size())
            return;
        const auto type = deleteFiles ? MessageID::DeleteProject : MessageID::RemoveRecentProject;
        if(!postProjectMessage(m_messageBus, type, m_descriptions[index])) {
            m_error = "Could not queue project removal.";
            return;
        }
        m_descriptions.erase(m_descriptions.begin() + index);
        if(m_configuration.selectedItem == static_cast<int>(index))
            m_configuration.selectedItem = -1;
        else if(m_configuration.selectedItem > static_cast<int>(index))
            --m_configuration.selectedItem;
        m_error.clear();
    }

} // namespace editor
