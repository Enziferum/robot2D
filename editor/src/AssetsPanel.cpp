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

#include <imgui/imgui.h>
#include <editor/AssetsPanel.hpp>
#include <robot2D/Util/Logger.hpp>

// TODO Move to FileManager API
#include <filesystem>
#include <stdexcept>

namespace editor {
    namespace fs = std::filesystem;

    // todo get it from Somewhere
    constexpr char* assetsPath = "assets";
    constexpr char* iconsPath = "res/icons/";

    const std::unordered_map<ResourceIconType, std::string> iconPaths = {
            {ResourceIconType::File, "FileIcon.png"},
            {ResourceIconType::Directory, "DirectoryIcon.png"}
    };

    AssetsPanel::AssetsPanel(): IPanel(UniqueType(typeid(AssetsPanel))),
    m_currentPath(assetsPath) {
        for(auto& it: iconPaths) {
            auto fullPath = iconsPath + it.second;
            if(!m_assetsIcons.loadFromFile(it.first, fullPath)) {
                std::runtime_error("Can't load Editor Icons");
            }
        }
    }

    void AssetsPanel::render() {
        ImGui::Begin("Assets");

        if(m_currentPath != fs::path(assetsPath)) {
            if(ImGui::Button("<-")) {
                m_currentPath = m_currentPath.parent_path();
            }
        }

        static float padding = 16.F;
        static float thumbNaleSize = 128.F;
        float cellSize = thumbNaleSize + padding;

        auto panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = int(panelWidth / cellSize);
        if(columnCount < 1)
            columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        for(auto& directoryEntry: fs::directory_iterator(m_currentPath)) {
            const auto& path = directoryEntry.path();
            auto relativePath = fs::relative(path, assetsPath);
            std::string filenameString = relativePath.filename().string();


            ImGui::PushID(filenameString.c_str());
            ResourceIconType iconType = directoryEntry.is_directory() ? ResourceIconType::Directory :
                    ResourceIconType::File;
            auto iconID = m_assetsIcons.get(iconType).getID();
            ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
            ImGui::ImageButton((void*)iconID, ImVec2{thumbNaleSize, thumbNaleSize},
                               {0, 1}, {1, 0});

            // drag&drop feature
            if(ImGui::BeginDragDropSource()) {
                const wchar_t* itemPath = relativePath.c_str();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t ));
                ImGui::EndDragDropSource();
            }

            ImGui::PopStyleColor();

            if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if(directoryEntry.is_directory())
                    m_currentPath /= path.filename();
            }
            ImGui::TextWrapped(filenameString.c_str());
            ImGui::NextColumn();

            ImGui::PopID();
        }

        ImGui::End();
    }

    void AssetsPanel::setAssetsPath(const std::string &path) {
        m_currentPath = fs::path(path);
    }
}