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
                {".scene", AssetsPanelConfiguration::ResourceIconType::Scene },
                {".cs", AssetsPanelConfiguration::ResourceIconType::Script },
                {".ttf", AssetsPanelConfiguration::ResourceIconType::Font },
                {".prefab", AssetsPanelConfiguration::ResourceIconType::Prefab },
                {".anim", AssetsPanelConfiguration::ResourceIconType::Animation },
        };

        const char* resourceIcon(AssetsPanelConfiguration::ResourceIconType type) {
            using Type = AssetsPanelConfiguration::ResourceIconType;
            switch(type) {
                case Type::Directory: return ICON_FA_FOLDER;
                case Type::Scene: return ICON_FA_LAYER_GROUP;
                case Type::Image: return ICON_FA_IMAGE;
                case Type::Script: return ICON_FA_CODE;
                case Type::Font: return ICON_FA_FONT;
                case Type::Prefab: return ICON_FA_CUBES;
                case Type::Animation: return ICON_FA_FILM;
                default: return ICON_FA_FILE;
            }
        }

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
                             PrefabManager& prefabManager):
    IPanel(UniqueType(typeid(AssetsPanel))),
    m_messageBus{messageBus},
    m_interactor{uiInteractor},
    m_uiManager(iuiManager),
    m_prefabManager{prefabManager},
    m_currentPath(""),
    m_configuration{} {

        m_itemEditName.second = false;
        setupIconAtlas();
        addObserverCallback(BIND_CLASS_FN(dropFiles));
    }

    void AssetsPanel::setupIconAtlas() {
        const auto& fontPath = m_configuration.iconFontPath;
        std::error_code error;
        if(!fs::is_regular_file(fontPath, error)) {
            RB_EDITOR_WARN("AssetsPanel: Can't load icon font: {0}", fontPath);
            return;
        }

        ImFontAtlas atlas;
        ImFontGlyphRangesBuilder builder;
        for(auto type: m_configuration.iconTypes)
            builder.AddText(resourceIcon(type));
        ImVector<ImWchar> ranges;
        builder.BuildRanges(&ranges);
        auto* font = atlas.AddFontFromFileTTF(fontPath.c_str(), 128.f, nullptr, ranges.Data);
        if(!font || !atlas.Build())
            return;
        unsigned char* pixels = nullptr;
        int width = 0, height = 0;
        atlas.GetTexDataAsRGBA32(&pixels, &width, &height);
        m_iconAtlas.create({static_cast<unsigned int>(width), static_cast<unsigned int>(height)}, pixels);
        for(auto type: m_configuration.iconTypes) {
            unsigned int codepoint = 0;
            ImTextCharFromUtf8(&codepoint, resourceIcon(type), nullptr);
            if(auto* glyph = font -> FindGlyphNoFallback(static_cast<ImWchar>(codepoint))) {
                m_resourceIcons[type] = {{glyph -> U0, glyph -> V0}, {glyph -> U1, glyph -> V1},
                                        {glyph -> X1 - glyph -> X0, glyph -> Y1 - glyph -> Y0}};
            }
        }
    }

    void AssetsPanel::drawResourceIcon(AssetsPanelConfiguration::ResourceIconType type,
                                       robot2D::vec2f position, float size) {
        auto* drawList = ImGui::GetWindowDrawList();
        const ImVec4 color = type == AssetsPanelConfiguration::ResourceIconType::Directory
                             ? ImVec4{0.55f, 0.76f, 0.96f, 1.f} : ImVec4{0.70f, 0.77f, 0.86f, 1.f};
        auto found = m_resourceIcons.find(type);
        if(found != m_resourceIcons.end()) {
            const auto& icon = found -> second;
            const float scale = size * 0.75f / std::max(icon.size.x, icon.size.y);
            const auto dimensions = icon.size * scale;
            const auto origin = position + (robot2D::vec2f{size, size} - dimensions) * 0.5f;
            drawList -> AddImage(ImGui::convertTextureHandle(m_iconAtlas.getID()),
                    {origin.x, origin.y}, {origin.x + dimensions.x, origin.y + dimensions.y},
                    {icon.uvMin.x, icon.uvMin.y}, {icon.uvMax.x, icon.uvMax.y}, ImGui::GetColorU32(color));
        }
        else {
            const char* glyph = resourceIcon(type);
            const auto dimensions = ImGui::CalcTextSize(glyph);
            drawList -> AddText({position.x + (size - dimensions.x) * 0.5f,
                                position.y + (size - dimensions.y) * 0.5f}, ImGui::GetColorU32(color), glyph);
        }
    }

    void AssetsPanel::setAssetsPath(const std::string& path) {
        m_assetsPath = fs::path(path);
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

            m_configuration.m_thumbnaleSize = static_cast<float>(EditorConfig::getConfig().assetsThumbnailSize);
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
                if(extension == ".png" || extension == ".jpg" ||  extension == ".ttf") {
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
        if (!m_itemClicked &&
            ImGui::BeginPopupContextWindow(nullptr,
                                           robot2D::mouse2int(robot2D::Mouse::MouseRight)))
        {
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
            ImGui::InvisibleButton("##NewAssetIcon", {m_configuration.m_thumbnaleSize, m_configuration.m_thumbnaleSize});
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
                auto toPath = m_currentPath;
                auto finalPath = combinePath(toPath.string(), fromPath.filename().string());
                fs::copy_file(fromPath, finalPath);
            } catch(fs::filesystem_error& e) {
                RB_EDITOR_ERROR("Assets Panel: Couldn't copy {0}", e.what());
            }
        }
    }

    void AssetsPanel::navigateTo(const std::filesystem::path& path) {
        m_currentPath = path;
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
        std::vector<fs::directory_entry> entries;
        for(fs::directory_iterator it(m_currentPath, error), end; !error && it != end; it.increment(error))
            entries.push_back(*it);
        std::stable_sort(entries.begin(), entries.end(), [](const auto& left, const auto& right) {
            std::error_code leftError, rightError;
            auto leftDirectory = left.is_directory(leftError);
            auto rightDirectory = right.is_directory(rightError);
            if(leftDirectory != rightDirectory)
                return leftDirectory;
            return left.path().filename() < right.path().filename();
        });
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
                    auto found = resourceTypes.find(path.extension().string());
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
                    ImGui::SetTooltip("%s", itemID.c_str());
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
                        else
                            m_itemEditName.first = relativePath.parent_path() / m_editName;
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
