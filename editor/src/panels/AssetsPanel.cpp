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

#include <filesystem>
#include <stdexcept>

#include <robot2D/Util/Logger.hpp>
#include <robot2D/imgui/Api.hpp>
#include <robot2D/Core/Mouse.hpp>

#include <editor/FileApi.hpp>
#include <editor/Messages.hpp>
#include <editor/panels/AssetsPanel.hpp>
#include <editor/Macro.hpp>
#include "editor/serializers/PrefabSerializer.hpp"
#include <editor/Uuid.hpp>
#include <editor/DragDropIDS.hpp>

namespace editor {
    namespace fs = std::filesystem;

    namespace {
        static ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue
                                                | ImGuiInputTextFlags_AutoSelectAll;

        static std::unordered_map< std::string, AssetsPanelConfiguration::ResourceIconType> resourceTypes = {
                {".png", AssetsPanelConfiguration::ResourceIconType::Image },
                {".scene", AssetsPanelConfiguration::ResourceIconType::Scene },
                {".cs", AssetsPanelConfiguration::ResourceIconType::Script },
                {".ttf", AssetsPanelConfiguration::ResourceIconType::Font },
                {".prefab", AssetsPanelConfiguration::ResourceIconType::Prefab },
        };
    }

    AssetsPanel::AssetsPanel(robot2D::MessageBus& messageBus, IUIManager& iuiManager, PrefabManager& prefabManager):
    IPanel(UniqueType(typeid(AssetsPanel))),
    m_messageBus{messageBus},
    m_uiManager(iuiManager),
    m_prefabManager{prefabManager},
    m_currentPath(""),
    m_configuration{} {

        for(auto& path: m_configuration.iconPaths) {
            auto fullPath = m_configuration.iconsPath + path.second;
            if(!m_assetsIcons.loadFromFile(path.first, fullPath)) {
                std::runtime_error("Can't load Editor's icons");
            }
        }

        m_itemEditName.second = false;
        addCallback(BIND_CLASS_FN(dropFiles));
    }

    void AssetsPanel::setAssetsPath(const std::string& path) {
        m_currentPath = fs::path(path);
        m_assetsPath = fs::path(path);
    }

    void AssetsPanel::render() {
        const float cellSize = m_configuration.m_thumbnaleSize + m_configuration.m_padding;
        static bool itemClicked = false;

        ImGui::Begin("Assets");

        if(!m_unlock) {
            ImGui::End();
            return;
        }

        if (!itemClicked && ImGui::BeginPopupContextWindow(nullptr, robot2D::mouse2int(robot2D::Mouse::MouseRight)))
        {
            if (ImGui::MenuItem("Add Folder")) {
                m_assetItems.emplace_back(AssetItem {AssetType::Folder, "Folder", m_currentPath,
                                                     fs::relative(m_currentPath, m_assetsPath)});
            }

            if(ImGui::MenuItem("Add Scene")) {
                m_assetItems.emplace_back(AssetItem{ AssetType::Scene , "New Scene.scene", m_currentPath,
                                                    fs::relative(m_currentPath, m_assetsPath)});
            }

            ImGui::EndPopup();
        }

        if(m_currentPath != fs::path(m_assetsPath)) {
            if(ImGui::Button("<-")) {
                m_currentPath = m_currentPath.parent_path();
                m_itemEditName.second = false;
                m_assetItems.clear();
            }
        }


        auto panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = int(panelWidth / cellSize);
        if(columnCount < 1)
            columnCount = 1;

        ImGui::Columns(columnCount, nullptr, false);

        for(auto& directoryEntry: fs::directory_iterator(m_currentPath)) {
            const auto& path = directoryEntry.path();
            auto relativePath = fs::relative(path, m_assetsPath);
            std::string filenameString = relativePath.filename().string();

            ImGui::PushID(filenameString.c_str());
            AssetsPanelConfiguration::ResourceIconType iconType = directoryEntry.is_directory() ?
                                                                  AssetsPanelConfiguration::ResourceIconType::Directory:
                                                                  AssetsPanelConfiguration::ResourceIconType::File;

            if(!directoryEntry.is_directory()) {
                auto extension = relativePath.extension();
                iconType = resourceTypes[extension.string()];
            }

            {
                robot2D::ScopedStyleColor scopedStyleColor(ImGuiCol_Button, robot2D::Color(255.f, 255.f, 255.f, 127.f));

                if(robot2D::ImageButton(m_assetsIcons[iconType], {m_configuration.m_thumbnaleSize,
                                                                  m_configuration.m_thumbnaleSize})) {
                    m_itemEditName.first = relativePath;
                    m_itemEditName.second = true;
                }
                if(ImGui::IsItemClicked(robot2D::mouse2int(robot2D::Mouse::MouseRight))) {
                    ImGui::OpenPopup("##Delete");
                    itemClicked = true;
                }

                if(ImGui::BeginPopup("##Delete")) {
                    if (ImGui::MenuItem("Delete")) {
                        try {
                            if(directoryEntry.is_directory()) {
                                if (!fs::remove_all(path)) {
                                    RB_EDITOR_WARN("AssetPanel: can't remove directory it's not exists");
                                }
                            }
                            else {
                                if(!fs::remove(path)) {
                                    RB_EDITOR_WARN("AssetPanel: can't remove file it's not exists");
                                }
                            }
                        }
                        catch(const std::exception& exception) {
                            RB_EDITOR_ERROR("AssetPanel Can't remove file, was exception thrown {0}",
                                            exception.what());
                        }

                        ImGui::CloseCurrentPopup();
                    }
                    itemClicked = false;
                    ImGui::EndPopup();
                }
            }

            if(ImGui::BeginDragDropTarget()) {
                auto* payload = ImGui::AcceptDragDropPayload("TreeNodeItem");
                if(payload) {
                    if(payload -> IsDataType("TreeNodeItem")) {
                        UUID id = *static_cast<UUID*>(payload -> Data);
                        auto entity = m_uiManager.getTreeItem(id);
                        if(entity) {
                            auto path = fs::relative(m_assetsPath);
                            bool ok = m_prefabManager.addPrefab(entity, path.string());
                            if(!ok) {
                                /// TODO(a.raag): can't create prefab
                            }
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if(ImGui::BeginDragDropSource()) {
                if(!directoryEntry.is_directory()) {
                    auto extension = relativePath.extension();
                    const wchar_t* itemPath = relativePath.c_str();

                    if(extension == ".scene" || extension == ".png" || extension == ".ttf") {
                        ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t ));
                        ImGui::Text(relativePath.filename().string().c_str());
                    }
                    if(extension == ".prefab") {
                        ImGui::SetDragDropPayload(contentPrefabItemID, itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t ));
                        ImGui::Text(relativePath.filename().string().c_str());
                    }

                    ImGui::EndDragDropSource();
                }
            }




            if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if(directoryEntry.is_directory()) {
                    m_currentPath /= path.filename();
                    m_assetItems.clear();
                }
            }

            if(m_itemEditName.first == relativePath && m_itemEditName.second) {
                if(robot2D::InputText("##ItemName", &filenameString, inputFlags)) {
                    auto rename_path = path;
                    rename_path.replace_filename(filenameString);
                    fs::rename(path, rename_path);
                    m_itemEditName.second = false;
                }
            }
            else {
                ImGui::TextWrapped(filenameString.c_str());
            }

            ImGui::NextColumn();
            ImGui::PopID();
        }

        for(auto asset: m_assetItems)
        {
            AssetsPanelConfiguration::ResourceIconType iconType;
            switch(asset.assetType) {
                case AssetType::Scene:
                    iconType = AssetsPanelConfiguration::ResourceIconType::Scene;
                    break;
                case AssetType::Folder:
                    iconType = AssetsPanelConfiguration::ResourceIconType::Directory;
                    break;
            }

            {
                robot2D::ScopedStyleColor scopedStyleColor(ImGuiCol_Button, robot2D::Color(255.f, 255.f, 255.f, 127.f));
                robot2D::ImageButton(m_assetsIcons[iconType], {m_configuration.m_thumbnaleSize,
                                                               m_configuration.m_thumbnaleSize});
            }


            ImGui::SetNextItemWidth(m_configuration.m_thumbnaleSize);
            if(robot2D::InputText("##ItemName", &asset.name, inputFlags)) {
                switch(asset.assetType) {
                    case AssetType::Scene: {
                        auto* msg = m_messageBus.postMessage<CreateSceneMessage>(MessageID::CreateScene);
                        auto pp = combinePath(asset.absolutePath.string(), asset.name);
                        msg -> path = pp;
                        break;
                    }
                    case AssetType::Folder: {
                        auto pp = combinePath(asset.absolutePath.string(), asset.name);
                        if(!createDirectory(pp)) {
                            /// TODO(a.raag): Log()
                        }
                        break;
                    }
                }
                m_assetItems.clear();
            }

            ImGui::NextColumn();
        }

        ImGui::End();
    }

    void AssetsPanel::dropFiles(std::vector<std::string>&& paths) {
        namespace fs = std::filesystem;
        for(const auto& path: paths) {
            try {
                fs::path fromPath{path};
                if(is_directory(fromPath))
                    continue;
                auto toPath = m_currentPath;
                auto finalPath = combinePath(toPath.string(), fromPath.filename().string());
                fs::copy_file(fromPath, finalPath);
            } catch(fs::filesystem_error& e) {
                RB_EDITOR_ERROR("Could not copy {0}", e.what());
            }
        }
    }

}