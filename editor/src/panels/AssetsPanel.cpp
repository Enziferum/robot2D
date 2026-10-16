/*********************************************************************
   (c) Alex Raag 2024
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
#include <algorithm>
#include <stdexcept>

#include <robot2D/Util/Logger.hpp>
#include <robot2D/imgui/Api.hpp>
#include <robot2D/imgui/Util.hpp>
#include <editor/EditorConfig.hpp>
#include <editor/EditorIcons.hpp>
#include <cctype>
#include <imgui/imgui_internal.h>
#include <robot2D/Core/Mouse.hpp>

#include <editor/FileApi.hpp>
#include <editor/Messages.hpp>
#include <editor/panels/AssetsPanel.hpp>
#include <editor/Macro.hpp>
#include <editor/serializers/PrefabSerializer.hpp>
#include <editor/Uuid.hpp>
#include <editor/DragDropIDS.hpp>
#include <editor/EditorResourceManager.hpp>
#include <editor/Buffer.hpp>

#include "../IconsFontsAwesome5.hpp"

namespace editor {
    namespace fs = std::filesystem;

    namespace {
        static ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue
            | ImGuiInputTextFlags_AutoSelectAll;

        static std::unordered_map< std::string, AssetsPanelConfiguration::ResourceIconType> resourceTypes = {
            {".png", AssetsPanelConfiguration::ResourceIconType::Image },
            {".jpg", AssetsPanelConfiguration::ResourceIconType::Image },
            {".jpeg", AssetsPanelConfiguration::ResourceIconType::Image },
            {".bmp", AssetsPanelConfiguration::ResourceIconType::Image },
            {".tga", AssetsPanelConfiguration::ResourceIconType::Image },
            {".scene", AssetsPanelConfiguration::ResourceIconType::Scene },
            {".cs", AssetsPanelConfiguration::ResourceIconType::Script },
            {".ttf", AssetsPanelConfiguration::ResourceIconType::Font },
            {".prefab", AssetsPanelConfiguration::ResourceIconType::Prefab },
            {".anim", AssetsPanelConfiguration::ResourceIconType::Animation },
        };

        void itemDeleteMenuEntry(const std::filesystem::directory_entry& entry) {
            if(ImGui::MenuItem("Delete")) {
                std::error_code error;
                if(entry.is_directory(error))
                    fs::remove_all(entry.path(), error);
                else
                    fs::remove(entry.path(), error);
                if(error)
                    RB_EDITOR_ERROR("AssetsPanel: Can't remove asset: {0}", error.message());
            }
        }
    }


    AssetsPanel::AssetsPanel(robot2D::MessageBus& messageBus,
        UIInteractor::Ptr uiInteractor,
        IUIManager& iuiManager,
        PrefabManager& prefabManager) :
        IPanel(UniqueType(typeid(AssetsPanel))),
        m_messageBus{messageBus},
        m_interactor{uiInteractor},
        m_uiManager(iuiManager),
        m_prefabManager{prefabManager},
        m_currentPath(""),
        m_configuration{} {

        m_itemEditName.second = false;
        addObserverCallback(BIND_CLASS_FN(dropFiles));
    }

    void AssetsPanel::drawResourceIcon(AssetsPanelConfiguration::ResourceIconType type,
        robot2D::vec2f position, float size) {
        static constexpr std::array<EditorIcon, 8> icons = {
            EditorIcon::File, EditorIcon::Scene, EditorIcon::Image, EditorIcon::Script,
            EditorIcon::Folder, EditorIcon::Font, EditorIcon::Prefab, EditorIcon::Animation
        };
        const ImVec4 color = type == AssetsPanelConfiguration::ResourceIconType::Directory
            ? ImVec4{0.55f, 0.76f, 0.96f, 1.f} : ImGui::GetStyleColorVec4(ImGuiCol_Text);
        const float side = size * 0.72f;
        drawEditorIcon(*ImGui::GetWindowDrawList(), icons[static_cast<std::size_t>(type)],
            {position.x + (size - side) * 0.5f, position.y + (size - side) * 0.5f}, side,
            ImGui::GetColorU32(color));
    }

    AssetPreview* AssetsPanel::drawImagePreview(const fs::path& path, robot2D::vec2f position, float size) {
        auto* preview = m_previews.request(path);
        if(!preview)
            return nullptr;
        auto* texture = m_previews.texture(*preview);
        if(!texture)
            return preview;
        const auto imageSize = texture -> getSize();
        if(imageSize.x == 0 || imageSize.y == 0)
            return preview;
        const float factor = size / static_cast<float>(std::max(imageSize.x, imageSize.y));
        const ImVec2 dimensions{imageSize.x * factor, imageSize.y * factor};
        const ImVec2 origin{position.x + (size - dimensions.x) * 0.5f,
                            position.y + (size - dimensions.y) * 0.5f};
        auto* draw = ImGui::GetWindowDrawList();
        for(int y = 0; y < static_cast<int>(dimensions.y); y += 8) {
            for(int x = 0; x < static_cast<int>(dimensions.x); x += 8) {
                const auto color = (x / 8 + y / 8) % 2 == 0 ? IM_COL32(52, 61, 69, 255) : IM_COL32(33, 41, 48, 255);
                draw -> AddRectFilled({origin.x + x, origin.y + y},
                    {origin.x + std::min(dimensions.x, static_cast<float>(x + 8)),
                     origin.y + std::min(dimensions.y, static_cast<float>(y + 8))}, color);
            }
        }
        draw -> AddImage(ImGui::convertTextureHandle(texture -> getID()), origin,
            {origin.x + dimensions.x, origin.y + dimensions.y});
        return preview;
    }

    void AssetsPanel::renderDirectoryTree(const fs::path& path, bool reveal, int depth) {
        if(depth > 32)
            return;
        const auto listing = m_browser.list(path);
        std::vector<fs::path> folders;
        for(const auto& entry: listing.entries) {
            std::error_code error;
            if(!entry.is_symlink(error) && entry.is_directory(error))
                folders.push_back(entry.path());
        }
        const bool root = path == m_assetsPath;
        const auto label = root ? std::string{"Assets"} : path.filename().u8string();
        const auto id = path.generic_u8string();
        ImGui::PushID(id.c_str());
        auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if(path == m_currentPath)
            flags |= ImGuiTreeNodeFlags_Selected;
        if(folders.empty())
            flags |= ImGuiTreeNodeFlags_Leaf;
        const auto relative = m_currentPath.lexically_relative(path);
        const bool ancestor = !relative.empty() && *relative.begin() != "..";
        if((root && reveal) || (reveal && ancestor))
            ImGui::SetNextItemOpen(true);
        const bool open = ImGui::TreeNodeEx("##Directory", flags, "    %s", label.c_str());
        const auto rect = ImGui::GetItemRectMin();
        drawEditorIcon(*ImGui::GetWindowDrawList(), EditorIcon::Folder,
            {rect.x + ImGui::GetTreeNodeToLabelSpacing(), rect.y + 2.f}, ImGui::GetFontSize(),
            ImGui::GetColorU32(ImVec4{0.55f, 0.76f, 0.96f, 1.f}));
        if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            navigateTo(path);
        if(!listing.error.empty() && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", listing.error.c_str());
        if(open) {
            for(const auto& folder: folders)
                renderDirectoryTree(folder, reveal, depth + 1);
            if(depth == 32 && !folders.empty())
                ImGui::TextDisabled("Use the card view to navigate deeper");
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void AssetsPanel::setAssetsPath(const std::string& path) {
        m_browser.setRoot(fs::path(path));
        m_assetsPath = m_browser.root();
        m_currentPath = m_browser.current();
        m_previews.setRoot(m_assetsPath);
        m_revealTree = true;
        navigateTo(m_assetsPath);
    }

    void AssetsPanel::render() {
        m_visible = false;

        bool anyItemIsHovered = false;


        std::string name = ICON_FA_FILE + std::string{" Assets"} + "###AssetsPanel";
        robot2D::WindowOptions options;
        options.flagsMask = 0;
        options.name = name;
        renderWindow(options, [&]() {
            m_visible = ImGui::IsWindowFocused();

            if(m_state == State::Loading)
                return;

            m_previews.beginFrame();
            if(m_assetsPath.empty() || !m_browser.recoverCurrent()) {
                ImGui::TextDisabled("Assets folder is unavailable");
                return;
            }
            if(m_currentPath != m_browser.current())
                navigateTo(m_browser.current());
            m_configuration.m_thumbnaleSize = static_cast<float>(EditorConfig::getConfig().assetsThumbnailSize);
            if(!ImGui::BeginTable("##AssetsLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
                return;
            ImGui::TableSetupColumn("Folders", ImGuiTableColumnFlags_WidthFixed, 180.f);
            ImGui::TableSetupColumn("Files", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextColumn();
            if(ImGui::BeginChild("##FolderTree", {0, 0})) {
                m_visible = m_visible || ImGui::IsWindowFocused();
                const bool reveal = m_revealTree;
                m_revealTree = false;
                renderDirectoryTree(m_assetsPath, reveal);
            }
            ImGui::EndChild();
            ImGui::TableNextColumn();
            ImGui::BeginChild("##AssetsContent", {0, 0});
            renderNavigation();
            ImGui::Separator();
            if(ImGui::BeginChild("##AssetBrowser", {0, 0})) {
                m_visible = m_visible || ImGui::IsWindowFocused();
                const float cellSize = std::max(96.f, m_configuration.m_thumbnaleSize + m_configuration.m_padding)
                + ImGui::GetStyle().CellPadding.x * 2.f;
                const auto panelWidth = ImGui::GetContentRegionAvail().x;
                const int tableCount = std::clamp(static_cast<int>(panelWidth / cellSize), 1, 64);
                std::error_code error;
                if(fs::is_directory(m_currentPath, error)) {
                    tableRender(tableCount, anyItemIsHovered);
                    if(fs::is_empty(m_currentPath, error) && m_assetItems.empty()) {
                        ImGui::TextDisabled("This folder is empty");
                        ImGui::Dummy({ImGui::GetContentRegionAvail().x,
                                      std::max(1.f, ImGui::GetContentRegionAvail().y)});
                        robot2D::DragDropTarget dragDropTarget{treeNodeItemID};
                        if(auto uuid = dragDropTarget.unpackPayload<UUID>()) {
                            auto entity = m_uiManager.getTreeItem(*uuid);
                            if(entity) {
                                auto path = fs::relative(m_currentPath);
                                if(!m_prefabManager.addPrefab(m_interactor, entity, path.string()))
                                    RB_EDITOR_ERROR("AssetsPanel: Can't add prefab by path: {0}", path.string());
                            }
                        }
                    }
                }
                else
                    ImGui::TextDisabled("Folder is unavailable");

                if(ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
                && !anyItemIsHovered && !ImGui::IsAnyItemHovered()) {
                    m_itemEditName = {};
                    m_itemClicked = false;
                }
                uiAssetsCreation();
            }
            ImGui::EndChild();
            ImGui::EndChild();
            ImGui::EndTable();
        });
    }

    void AssetsPanel::processDragDrop(const std::filesystem::directory_entry& directoryEntry,
        std::filesystem::path& relativePath) {
        {
            robot2D::DragDropTarget dragDropTarget{treeNodeItemID};
            if(auto* uuid = dragDropTarget.unpackPayload<UUID>()) {
                auto entity = m_uiManager.getTreeItem(*uuid);
                if(entity) {
                    auto path = fs::relative(directoryEntry.is_directory() ? directoryEntry.path() : m_currentPath);



                    bool success = m_prefabManager.addPrefab(m_interactor, entity, path.string());
                    if(!success)
                        RB_EDITOR_ERROR("AssetPanel: Can't add prefab by path: {0}", path.string());
                }
            }
        }

        imgui_DragDropSource() {
            if(!directoryEntry.is_directory()) {
                auto extension = relativePath.extension();
                const auto pathString = relativePath.string();
                const char* itemPath = pathString.c_str();
                const auto len = pathString.size() + 1;

                if(extension == ".scene") {
                    ImGui::SetDragDropPayload(contentSceneID, itemPath, len);
                    ImGui::TextUnformatted(relativePath.filename().string().c_str());
                }
                if(extension == ".png" || extension == ".jpg" || extension == ".ttf") {
                    ImGui::SetDragDropPayload(contentItemID, itemPath, len);
                    ImGui::TextUnformatted(relativePath.filename().string().c_str());
                }
                if(extension == ".prefab") {
                    ImGui::SetDragDropPayload(contentPrefabItemID, itemPath, len);
                    ImGui::TextUnformatted(relativePath.filename().string().c_str());
                }

            }
        }
    }

    void AssetsPanel::uiAssetsCreation() {
        if(!m_itemClicked &&
            ImGui::BeginPopupContextWindow(nullptr,
            robot2D::mouse2int(robot2D::Mouse::MouseRight))) {
            m_itemEditName.second = false;
            imgui_MenuItem("Add Folder") {
                m_assetItems.emplace_back(AssetItem {AssetType::Folder,
                                                     "Folder",
                                                     m_currentPath,
                                                     fs::relative(m_currentPath, m_assetsPath)});
            }

            imgui_MenuItem("Add Scene") {
                m_assetItems.emplace_back(AssetItem{ AssetType::Scene,
                                                     "New Scene.scene",
                                                     m_currentPath,
                                                     fs::relative(m_currentPath, m_assetsPath)});
            }

            ImGui::EndPopup();
        }

    }

    void AssetsPanel::processAssets() {
        for(std::size_t index = 0; index < m_assetItems.size(); ++index) {
            auto& asset = m_assetItems[index];
            ImGui::TableNextColumn();
            ImGui::PushID(static_cast<int>(index));
            auto iconType = asset.assetType == AssetType::Scene
                            ? AssetsPanelConfiguration::ResourceIconType::Scene
                            : AssetsPanelConfiguration::ResourceIconType::Directory;
            const auto iconPosition = ImGui::GetCursorScreenPos();
            ImGui::InvisibleButton("##NewAssetIcon",
                {m_configuration.m_thumbnaleSize, m_configuration.m_thumbnaleSize});
            drawResourceIcon(iconType, {iconPosition.x, iconPosition.y}, m_configuration.m_thumbnaleSize);
            ImGui::SetNextItemWidth(m_configuration.m_thumbnaleSize);
            const bool submitted = robot2D::InputText("##NewItemName", &asset.name, inputFlags);
            ImGui::PopID();
            if(submitted) {
                if(asset.name.empty() || fs::path(asset.name).has_parent_path()
                    || asset.name == "." || asset.name == "..")
                    continue;
                auto fullPath = combinePath(asset.absolutePath.string(), asset.name);
                if(asset.assetType == AssetType::Scene) {
                    auto* msg = m_messageBus.postMessage<CreateSceneMessage>(MessageID::CreateScene);
                    msg -> path = fullPath;
                }
                else if(!createDirectory(fullPath))
                    RB_EDITOR_ERROR("Assets Panel: Can't create folder by path {0}", fullPath);
                m_assetItems.clear();
                break;
            }
        }
    }
    void AssetsPanel::dropFiles(const std::vector<std::string>& paths) {
        if(getState() != PanelState::Visible || !m_visible)
            return;

        namespace fs = std::filesystem;
        for(const auto& path: paths) {
            try {
                fs::path fromPath{path};
                if(is_directory(fromPath))
                    continue;
                importProjectAsset(m_assetsPath.parent_path().u8string(), path,
                    m_currentPath.u8string());
            } catch(const std::exception& e) {
                RB_EDITOR_ERROR("Assets import failed; source='{}'; destination='{}'; reason='{}'",
                    path, m_currentPath.u8string(), e.what());
            }
        }
    }

    void AssetsPanel::navigateTo(const std::filesystem::path& path) {
        if(!m_browser.navigate(path))
            return;
        m_currentPath = m_browser.current();
        m_revealTree = true;
        m_itemEditName = {};
        m_itemClicked = false;
        m_editName.clear();
        m_assetItems.clear();
    }

    void AssetsPanel::renderNavigation() {
        ImGui::BeginDisabled(m_currentPath == m_assetsPath);
        if(ImGui::Button(ICON_FA_ARROW_UP "##ParentFolder"))
            navigateTo(m_currentPath.parent_path());
        if(ImGui::IsItemHovered())
            ImGui::SetTooltip("Parent folder");
        ImGui::EndDisabled();
        ImGui::SameLine();
        if(ImGui::Button(ICON_FA_SYNC_ALT "##RefreshAssets")) {
            m_browser.refresh();
            m_previews.clear();
        }
        if(ImGui::IsItemHovered())
            ImGui::SetTooltip("Refresh folders and previews");
        ImGui::SameLine();
        const bool wide = ImGui::GetContentRegionAvail().x > 400.f;
        const float pathWidth = std::max(1.f, ImGui::GetContentRegionAvail().x - (wide ? 180.f : 0.f));
        const float height = ImGui::GetFrameHeight() + ImGui::GetStyle().ScrollbarSize;
        fs::path destination;
        if(ImGui::BeginChild("##AssetPath", {pathWidth, height}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
            if(ImGui::SmallButton("Assets"))
                destination = m_assetsPath;
            auto path = m_assetsPath;
            auto relative = m_currentPath.lexically_relative(m_assetsPath);
            for(const auto& segment: relative) {
                if(segment == ".")
                    continue;
                path /= segment;
                ImGui::SameLine();
                ImGui::TextDisabled("/");
                ImGui::SameLine();
                const auto id = path.string();
                ImGui::PushID(id.c_str());
                if(ImGui::SmallButton(segment.string().c_str()))
                    destination = path;
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        if(!destination.empty())
            navigateTo(destination);
        if(wide)
            ImGui::SameLine();
        ImGui::SetNextItemWidth(std::min(160.f, ImGui::GetContentRegionAvail().x));
        auto& thumbnailSize = EditorConfig::getConfig().assetsThumbnailSize;
        if(ImGui::SliderInt("##PreviewSize", &thumbnailSize, 48, 128, "Preview: %d px",
            ImGuiSliderFlags_AlwaysClamp)) {
            m_configuration.m_thumbnaleSize = static_cast<float>(thumbnailSize);
            ImGui::GetIO().WantSaveIniSettings = true;
        }
        if(ImGui::IsItemHovered())
            ImGui::SetTooltip("Asset preview size");
    }

    void AssetsPanel::tableRender(int tableCount, bool& anyItemIsHovered) {
        std::error_code error;
        const auto listing = m_browser.list(m_currentPath);
        const auto& entries = listing.entries;
        if(!listing.error.empty())
            ImGui::TextDisabled("Folder error: %s", listing.error.c_str());
        fs::path nextPath;
        if(ImGui::BeginTable("##AssetCards", tableCount, ImGuiTableFlags_SizingStretchSame)) {
            for(const auto& directoryEntry: entries) {
                ImGui::TableNextColumn();
                const auto path = directoryEntry.path();
                auto relativePath = path.lexically_relative(m_assetsPath);
                const auto filename = path.filename().string();
                const auto itemID = relativePath.string();
                const bool directory = directoryEntry.is_directory(error);
                auto iconType = directory ? AssetsPanelConfiguration::ResourceIconType::Directory
                                          : AssetsPanelConfiguration::ResourceIconType::File;
                if(!directory) {
                    auto extension = path.extension().string();
                    std::transform(extension.begin(), extension.end(), extension.begin(),
                        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                    auto found = resourceTypes.find(extension);
                    if(found != resourceTypes.end())
                        iconType = found -> second;
                }
                ImGui::PushID(itemID.c_str());
                const auto position = ImGui::GetCursorScreenPos();
                const float width = std::max(1.f, ImGui::GetContentRegionAvail().x);
                const float preview = std::max(1.f, std::min(m_configuration.m_thumbnaleSize, width - 12.f));
                const float height = preview + ImGui::GetTextLineHeight() * 2.f + 14.f;
                const bool selected = m_itemEditName.first == relativePath;
                bool focusRename = false;
                auto* drawList = ImGui::GetWindowDrawList();
                drawList -> AddRectFilled(position, {position.x + width, position.y + height},
                    ImGui::GetColorU32(ImGuiCol_FrameBg), 4.f);
                const bool clicked = ImGui::Selectable("##Asset", selected,
                        ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowItemOverlap, {width, height});
                const bool hovered = ImGui::IsItemHovered();
                const bool rightClicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
                const bool doubleClicked = hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
                anyItemIsHovered |= hovered || ImGui::IsItemFocused();
                if((clicked && !(selected && m_itemEditName.second)) || rightClicked) {
                    m_itemEditName = {relativePath, false};
                    m_itemClicked = rightClicked;
                    if(clicked && path.extension() == ".prefab") {
                        int allocSize = StringBuffer::calcAllocSize(itemID);
                        Buffer buffer{m_messageBus.postMessage(MessageID::PrefabAssetPressed, allocSize)};
                        pack_message_string(itemID, buffer);
                    }
                }
                AssetPreview* thumbnail = nullptr;
                if(!directory && iconType == AssetsPanelConfiguration::ResourceIconType::Image
                    && ImGui::IsRectVisible(position, {position.x + width, position.y + height}))
                    thumbnail = drawImagePreview(path,
                            {position.x + (width - preview) * 0.5f, position.y + 5.f}, preview);
                if(!thumbnail || thumbnail -> status != PreviewStatus::Ready)
                    drawResourceIcon(iconType, {position.x + (width - preview) * 0.5f, position.y + 5.f}, preview);
                const float textY = position.y + preview + 8.f;
                drawList -> PushClipRect(position, {position.x + width - 3.f, position.y + height}, true);
                if(!(selected && m_itemEditName.second))
                    ImGui::RenderTextEllipsis(drawList, {position.x + 4.f, textY},
                        {position.x + width - 4.f, textY + ImGui::GetTextLineHeight()},
                        position.x + width - 4.f, position.x + width - 4.f,
                        filename.c_str(), nullptr, nullptr);
                const auto type = directory ? std::string{"Folder"}
                                            : (path.has_extension() ? path.extension().string() : "File");
                drawList -> AddText({position.x + 4.f, textY + ImGui::GetTextLineHeight()},
                    ImGui::GetColorU32(ImGuiCol_TextDisabled), type.c_str());
                drawList -> PopClipRect();

                processDragDrop(directoryEntry, relativePath);
                if(doubleClicked && directory)
                    nextPath = path;
                if(hovered && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                    ImGui::SetTooltip("%s%s%s", itemID.c_str(),
                        thumbnail && !thumbnail -> error.empty() ? "\nPreview: " : "",
                        thumbnail ? thumbnail -> error.c_str() : "");
                if(rightClicked)
                    ImGui::OpenPopup("##AssetActions");
                if(ImGui::BeginPopup("##AssetActions")) {
                    if(ImGui::MenuItem("Rename", "F2")) {
                        m_itemEditName = {relativePath, true};
                        m_editName = filename;
                        focusRename = true;
                    }
                    itemDeleteMenuEntry(directoryEntry);
                    ImGui::EndPopup();
                }
                if(selected && ImGui::IsWindowFocused() && !ImGui::GetIO().WantTextInput
                    && ImGui::IsKeyPressed(ImGuiKey_F2)) {
                    m_itemEditName.second = true;
                    m_editName = filename;
                    focusRename = true;
                }
                if(m_itemEditName.first == relativePath && m_itemEditName.second) {
                    const auto afterCard = ImGui::GetCursorScreenPos();
                    ImGui::SetCursorScreenPos({position.x + 2.f, textY});
                    ImGui::SetNextItemWidth(std::max(1.f, width - 4.f));
                    if(focusRename)
                        ImGui::SetKeyboardFocusHere();
                    if(robot2D::InputText("##ItemName", &m_editName, inputFlags)
                        && !m_editName.empty() && !fs::path(m_editName).has_parent_path()
                        && m_editName != "." && m_editName != "..") {
                        fs::rename(path, path.parent_path() / m_editName, error);
                        if(error)
                            RB_EDITOR_ERROR("AssetsPanel: Can't rename asset: {0}", error.message());
                        else {
                            m_previews.invalidate(path);
                            m_browser.refresh();
                            m_itemEditName.first = relativePath.parent_path() / m_editName;
                        }
                        m_itemEditName.second = false;
                    }
                    if(ImGui::IsKeyPressed(ImGuiKey_Escape))
                        m_itemEditName.second = false;
                    anyItemIsHovered |= ImGui::IsItemHovered() || ImGui::IsItemActive();
                    ImGui::SetCursorScreenPos(afterCard);
                }
                ImGui::PopID();
            }
            processAssets();
            ImGui::EndTable();
        }
        if(!nextPath.empty())
            navigateTo(nextPath);
    }

}
