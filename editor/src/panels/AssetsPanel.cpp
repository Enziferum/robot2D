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
#include <stdexcept>

#include <robot2D/Util/Logger.hpp>
#include <robot2D/imgui/Api.hpp>
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
                {".anim", AssetsPanelConfiguration::ResourceIconType::Animation },
        };


        void itemDeletePopUp(const std::filesystem::directory_entry& directoryEntry,
                             const std::filesystem::path& path) {

            imgui_Popup("##Delete") {
                imgui_MenuItem("Delete") {
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
            }
        }

/*
        bool BeginDrapDropTargetWindow(const char* payload_type)
        {
            using namespace ImGui;
            ImRect inner_rect = GetCurrentWindow() -> InnerRect;
            if (BeginDragDropTargetCustom(inner_rect, GetID("##WindowBgArea")))
                if (const ImGuiPayload* payload = AcceptDragDropPayload(payload_type,
                                                                        ImGuiDragDropFlags_AcceptBeforeDelivery
                                                                        | ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
                {
                    if (payload->IsPreview())
                    {
                        ImDrawList* draw_list = GetForegroundDrawList();
                        draw_list->AddRectFilled(inner_rect.Min, inner_rect.Max, GetColorU32(ImGuiCol_DragDropTarget, 0.05f));
                        draw_list->AddRect(inner_rect.Min, inner_rect.Max, GetColorU32(ImGuiCol_DragDropTarget), 0.0f, 0, 2.0f);
                    }
                    if (payload -> IsDelivery())
                        return true;
                    EndDragDropTarget();
                }
            return false;
        }

        class DragDropTarget {
        public:
            explicit DragDropTarget(const std::string& id);
            DragDropTarget() = delete;
            DragDropTarget(const DragDropTarget& other) = delete;
            DragDropTarget& operator=(const DragDropTarget& other) = delete;
            DragDropTarget(DragDropTarget&& other) = delete;
            DragDropTarget& operator=(DragDropTarget&& other) = delete;
            ~DragDropTarget();

            template<typename T>
            T* unpackPayload();
        private:
            const std::string& m_id;
            bool m_beginTarget { false };
        };

        DragDropTarget::DragDropTarget(const std::string& id): m_id(id) {
            if(BeginDrapDropTargetWindow(m_id.c_str()))
                m_beginTarget = true;
        }

        DragDropTarget::~DragDropTarget() {
            if(m_beginTarget)
                ImGui::EndDragDropTarget();
        }

        template<typename T>
        inline T* DragDropTarget::unpackPayload() {
            if(!m_beginTarget)
                return nullptr;
            auto* payload = ImGui::AcceptDragDropPayload(m_id.c_str());
            if(!payload || !payload -> IsDataType(m_id.c_str()))
                return nullptr;
            return static_cast<T*>(payload -> Data);
        }*/
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

        for(auto& path: m_configuration.iconPaths) {
            auto fullPath = m_configuration.iconsPath + path.second;
            if(!m_assetsIcons.loadFromFile(path.first, fullPath)) {
                std::runtime_error("Can't load Editor's icons");
            }
        }

        m_itemEditName.second = false;
        addObserverCallback(BIND_CLASS_FN(dropFiles));
    }

    void AssetsPanel::setAssetsPath(const std::string& path) {
        m_currentPath = fs::path(path);
        m_assetsPath = fs::path(path);
    }

    void AssetsPanel::render() {
        const float cellSize = m_configuration.m_thumbnaleSize + m_configuration.m_padding;
        bool anyItemIsHovered = false;

        imgui_Window("Assets") {
            m_visible = ImGui::IsWindowFocused();

            if(m_state == State::Loading)
                return;

            if(m_currentPath != fs::path(m_assetsPath)) {
                if(ImGui::Button("<-")) {
                    m_currentPath = m_currentPath.parent_path();
                    m_itemEditName.second = false;
                    m_assetItems.clear();
                }
            }

            auto panelWidth = ImGui::GetContentRegionAvail().x;
            int tableCount = int(panelWidth / cellSize);
            if(tableCount < 1)
                tableCount = 1;

            if(is_directory(m_currentPath) && is_empty(m_currentPath)) {
                robot2D::DragDropTarget dragDropTarget{treeNodeItemID};
                if(auto uuid = dragDropTarget.unpackPayload<UUID>()) {
                    auto entity = m_uiManager.getTreeItem(*uuid);
                    if(entity) {
                        auto path = fs::relative(m_currentPath);
                        bool ok = m_prefabManager.addPrefab(m_interactor, entity, path.string());
                        if(!ok) {
                            RB_EDITOR_ERROR("AssetPanel: Can't add prefab by path: {0}", path.string());
                        }
                    }
                }
            }
            else {
                imgui_Table("MyTable", tableCount) {
                    for(auto& directoryEntry: fs::directory_iterator(m_currentPath)) {
                        ImGui::TableNextColumn();
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
                            robot2D::ScopedStyleColor scopedStyleColor(ImGuiCol_Button,
                                                                       robot2D::Color(255.f, 255.f, 255.f, 127.f));

                            if(robot2D::ImageButton(m_assetsIcons[iconType], {m_configuration.m_thumbnaleSize,
                                                                              m_configuration.m_thumbnaleSize})) {
                                m_itemEditName.first = relativePath;
                                m_itemEditName.second = true;

                                auto extension = relativePath.extension();
                                if(extension == ".prefab") {
                                    auto str = relativePath.string();
                                    int allocSize = StringBuffer::calcAllocSize(str);
                                    void* rawBuffer = m_messageBus.postMessage(MessageID::PrefabAssetPressed, allocSize);
                                    Buffer buffer { rawBuffer };
                                    pack_message_string(str, buffer);
                                }

                            }
                            if(ImGui::IsItemClicked(robot2D::mouse2int(robot2D::Mouse::MouseRight))) {
                                ImGui::OpenPopup("##Delete");
                                m_itemClicked = true;
                                m_itemEditName.second = false;
                            }

                            itemDeletePopUp(directoryEntry, path);
                        }

                        processDragDrop(directoryEntry, relativePath);

                        if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                            if(directoryEntry.is_directory()) {
                                m_currentPath /= path.filename();
                                m_assetItems.clear();
                            }
                        }

                        if(m_itemEditName.first == relativePath && m_itemEditName.second) {
                            imgui_InputText("##ItemName", &filenameString, inputFlags) {
                                auto rename_path = path;
                                rename_path.replace_filename(filenameString);
                                fs::rename(path, rename_path);
                                m_itemEditName.second = false;
                            }
                        }
                        else {
                            ImGui::TextWrapped("%s", filenameString.c_str());
                        }

                        if(ImGui::IsItemHovered() || ImGui::IsItemFocused()) {
                            anyItemIsHovered = true;
                        }

                        ImGui::PopID();
                    }
                }

            }

            if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !anyItemIsHovered) {
                m_itemEditName.second = false;
                m_itemClicked = false;
            }

            uiAssetsCreation();
            processAssets();
        }
    }

    void AssetsPanel::processDragDrop(const std::filesystem::directory_entry& directoryEntry,
                                      std::filesystem::path& relativePath) {
        {
            robot2D::DragDropTarget dragDropTarget{treeNodeItemID};
            if(auto* uuid = dragDropTarget.unpackPayload<UUID>()) {
                auto entity = m_uiManager.getTreeItem(*uuid);
                if(entity) {
                    auto path = fs::relative(m_assetsPath);



                    bool success = m_prefabManager.addPrefab(m_interactor, entity, path.string());
                    if(!success)
                        RB_EDITOR_ERROR("AssetPanel: Can't add prefab by path: {0}", path.string());
                }
            }
        }


        imgui_DragDropSource() {
            if(!directoryEntry.is_directory()) {
                auto extension = relativePath.extension();
#ifdef ROBOT2D_WINDOWS
                const char* itemPath = relativePath.string().c_str(); /// ImGui::SetDragDropPayload copy buffer, so it's ok.
                auto len = (strlen(itemPath) + 1) * sizeof(char);
#else
                const char* itemPath = relativePath.c_str();
                auto len = (strlen(itemPath) + 1) * sizeof(char);
#endif

                if(extension == ".scene") {
                    ImGui::SetDragDropPayload(contentSceneID, itemPath, len);
                    imgui_Text(relativePath.filename().string().c_str());
                }
                if(extension == ".png" || extension == ".ttf") {
                   ImGui::SetDragDropPayload(contentItemID, itemPath, len);
                   imgui_Text(relativePath.filename().string().c_str());
                }
                if(extension == ".prefab") {
                    ImGui::SetDragDropPayload(contentPrefabItemID, itemPath, len);
                    imgui_Text(relativePath.filename().string().c_str());
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
                robot2D::ScopedStyleColor scopedStyleColor(ImGuiCol_Button,
                                                           robot2D::Color(255.f, 255.f, 255.f, 127.f));
                robot2D::ImageButton(m_assetsIcons[iconType],
                                     {m_configuration.m_thumbnaleSize,
                                      m_configuration.m_thumbnaleSize}
                );
            }


            ImGui::SetNextItemWidth(m_configuration.m_thumbnaleSize);
            imgui_InputText("##ItemName", &asset.name, inputFlags) {
                switch(asset.assetType) {
                    case AssetType::Scene: {
                        auto* msg = m_messageBus.postMessage<CreateSceneMessage>(MessageID::CreateScene);
                        auto fullPath = combinePath(asset.absolutePath.string(), asset.name);
                        msg -> path = fullPath;
                        break;
                    }
                    case AssetType::Folder: {
                        auto fullPath = combinePath(asset.absolutePath.string(), asset.name);
                        if(!createDirectory(fullPath))
                            RB_EDITOR_ERROR("Assets Panel: Can't create folder by path {0}", fullPath.c_str());
                        break;
                    }
                }
                m_assetItems.clear();
            }

            ImGui::NextColumn();
        }
    }

    void AssetsPanel::dropFiles(const std::vector<std::string>& paths) {
        if(!m_visible)
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

}