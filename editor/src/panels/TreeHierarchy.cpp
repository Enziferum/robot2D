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

#include <imgui/imgui.h>
#include <editor/Components.hpp>
#include <editor/components/ButtonComponent.hpp>
#include <editor/panels/TreeHierarchy.hpp>

#include "../IconsFontsAwesome5.hpp"

namespace editor {

    namespace {
        constexpr const char* treePayloadIdentifier = "TreeNodeItem";

        struct EntityIcon {
            const char* glyph;
            const char* type;
            ImVec4 color;
        };

        EntityIcon getEntityIcon(SceneEntity entity) {
            if(entity.hasComponent<CameraComponent>())
                return {ICON_FA_CAMERA, "Camera", {0.55f, 0.76f, 0.96f, 1.f}};
            if(entity.hasComponent<ButtonComponent>())
                return {ICON_FA_MOUSE_POINTER, "Button", {0.62f, 0.82f, 0.72f, 1.f}};
            if(entity.hasComponent<TextComponent>())
                return {ICON_FA_FONT, "Text", {0.85f, 0.78f, 0.58f, 1.f}};
            if(entity.hasComponent<PrefabComponent>())
                return {ICON_FA_CUBES, "Prefab", {0.67f, 0.65f, 0.91f, 1.f}};
            if(entity.hasComponent<DrawableComponent>())
                return {ICON_FA_IMAGE, "Drawable", {0.62f, 0.82f, 0.72f, 1.f}};
            if(entity.hasComponent<ScriptComponent>())
                return {ICON_FA_CODE, "Script", {0.85f, 0.78f, 0.58f, 1.f}};
            return {ICON_FA_CUBE, "Entity", ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)};
        }

        bool renderTreeRow(const char* id, ImGuiTreeNodeFlags flags,
                           const EntityIcon& icon, const std::string& name) {
            const auto cursor = ImGui::GetCursorScreenPos();
            const float labelX = cursor.x + ImGui::GetTreeNodeToLabelSpacing();
            const bool open = ImGui::TreeNodeEx(id, flags, " ");
            const auto rowMin = ImGui::GetItemRectMin();
            const auto rowMax = ImGui::GetItemRectMax();
            const float textY = rowMin.y + (rowMax.y - rowMin.y - ImGui::GetFontSize()) * 0.5f;
            const float iconWidth = ImGui::GetFontSize() + 6.f;
            auto* drawList = ImGui::GetWindowDrawList();
            drawList -> PushClipRect(rowMin, rowMax, true);
            drawList -> AddText({labelX, textY}, ImGui::GetColorU32(icon.color), icon.glyph);
            drawList -> AddText({labelX + iconWidth, textY}, ImGui::GetColorU32(ImGuiCol_Text), name.c_str());
            drawList -> PopClipRect();
            if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(name.c_str());
                ImGui::TextDisabled("%s", icon.type);
                ImGui::EndTooltip();
            }
            return open;
        }
    }

    TreeHierarchy::TreeHierarchy(std::string name): m_name(std::move(name)) {}

    void TreeHierarchy::setEntitySource(EntitySource&& source) {
        m_entitySource = std::move(source);
    }

    void TreeHierarchy::setSelectionService(SelectionService& selectionService) {
        m_selectionService = &selectionService;
    }

    void TreeHierarchy::setSelectionCallback(SelectionCallback&& callback) {
        m_selectionCallback = std::move(callback);
    }

    void TreeHierarchy::setReorderCallback(ReorderCallback&& callback) {
        m_reorderCallback = std::move(callback);
    }

    void TreeHierarchy::setMakeChildCallback(ReorderCallback&& callback) {
        m_makeChildCallback = std::move(callback);
    }

    void TreeHierarchy::setExpanded(UUID uuid, bool expanded) {
        m_nodeStates[uuid].expanded = expanded;
    }

    bool TreeHierarchy::isExpanded(UUID uuid) const {
        auto found = m_nodeStates.find(uuid);
        return found == m_nodeStates.end() || found -> second.expanded;
    }

    void TreeHierarchy::clear() {
        m_nodeStates.clear();
        m_rootExpanded = true;
        m_rangeAnchor.reset();
        m_clickedEntity.reset();
        m_visibleEntities.clear();
        m_pendingDrops.clear();
    }

    void TreeHierarchy::clearSelection() {
        if(m_selectionService)
            m_selectionService -> clear();
        m_rangeAnchor.reset();
        notifySelection();
    }

    void TreeHierarchy::notifySelection() {
        if(m_selectionCallback)
            m_selectionCallback();
    }

    void TreeHierarchy::render() {
        m_visibleEntities.clear();
        m_clickedEntity.reset();
        m_pendingDrops.clear();
        if(!m_entitySource)
            return;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4.f, 3.f});
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {4.f, 1.f});
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() + 4.f);
        ImGui::SetNextItemOpen(m_rootExpanded, ImGuiCond_Always);
        m_rootExpanded = renderTreeRow(m_name.c_str(),
                ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
                | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding,
                {ICON_FA_LAYER_GROUP, "Scene", ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)}, m_name);
        if(m_rootExpanded) {
            // Copy descriptors for this frame; hierarchy changes are applied after traversal.
            auto roots = m_entitySource();
            for(auto entity: roots)
                renderEntity(entity);
            ImGui::TreePop();
        }
        ImGui::PopStyleVar(3);

        processSelection();
        processDrops();
    }

    void TreeHierarchy::renderEntity(SceneEntity entity) {
        if(!entity || !entity.hasComponent<IDComponent>())
            return;
        auto uuid = entity.getUUID();
        auto children = entity.getChildren();
        auto expanded = isExpanded(uuid);
        auto selected = m_selectionService && m_selectionService -> hasEntity(uuid);
        auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
                     | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding
                     | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if(children.empty())
            flags |= ImGuiTreeNodeFlags_Leaf;
        if(selected)
            flags |= ImGuiTreeNodeFlags_Selected;

        // UUID determines the widget identity independently of its name and parent.
        auto id = std::to_string(static_cast<std::uint64_t>(uuid));
        ImGui::PushID(id.c_str());
        ImGui::SetNextItemOpen(expanded, ImGuiCond_Always);
        auto name = entity.hasComponent<TagComponent>() ? entity.getName() : "Untitled Entity";
        const auto icon = getEntityIcon(entity);
        bool nodeOpen = renderTreeRow("##Entity", flags, icon, name);
        m_visibleEntities.emplace_back(uuid);
        if(!children.empty())
            m_nodeStates[uuid].expanded = nodeOpen;

        if(ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
            m_clickedEntity = uuid;

        if(ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload(treePayloadIdentifier, &uuid, sizeof(uuid));
            ImGui::Text("%s  %s", icon.glyph, name.c_str());
            ImGui::EndDragDropSource();
        }
        if(ImGui::BeginDragDropTarget()) {
            if(auto* payload = ImGui::AcceptDragDropPayload(treePayloadIdentifier)) {
                if(payload -> DataSize == sizeof(UUID)) {
                    auto source = *static_cast<const UUID*>(payload -> Data);
                    if(source != uuid) {
                        auto action = entity.isChild() || ImGui::IsKeyDown(ImGuiKey_Q)
                                      ? DropAction::MakeChild : DropAction::Reorder;
                        m_pendingDrops.push_back({source, uuid, action});
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::PopID();

        if(nodeOpen && !children.empty()) {
            ImGui::Indent();
            for(auto child: children)
                renderEntity(child);
            ImGui::Unindent();
        }
    }

    void TreeHierarchy::processSelection() {
        if(!m_selectionService)
            return;
        const auto& io = ImGui::GetIO();
        if(m_clickedEntity) {
            auto uuid = *m_clickedEntity;
            bool rangeSelected = false;
            if(io.KeyShift && m_rangeAnchor) {
                auto first = std::find(m_visibleEntities.begin(), m_visibleEntities.end(), *m_rangeAnchor);
                auto last = std::find(m_visibleEntities.begin(), m_visibleEntities.end(), uuid);
                if(first != m_visibleEntities.end() && last != m_visibleEntities.end()) {
                    if(last < first)
                        std::swap(first, last);
                    std::vector<UUID> selectedEntities;
                    if(io.KeyCtrl)
                        selectedEntities = m_selectionService -> getSelectedEntities();
                    selectedEntities.insert(selectedEntities.end(), first, std::next(last));
                    m_selectionService -> selectEntities(selectedEntities, uuid);
                    rangeSelected = true;
                }
            }
            if(!rangeSelected) {
                if(io.KeyCtrl)
                    m_selectionService -> toggleEntity(uuid);
                else
                    m_selectionService -> selectEntity(uuid);
                m_rangeAnchor = uuid;
            }
            notifySelection();
        }
        if(ImGui::IsWindowFocused() && !io.WantTextInput) {
            if(ImGui::IsKeyPressed(ImGuiKey_Escape))
                clearSelection();
            else if(io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A, false)) {
                m_selectionService -> selectEntities(m_visibleEntities);
                m_rangeAnchor = m_selectionService -> lastActive();
                notifySelection();
            }
        }
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()
           && !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())
            clearSelection();
    }

    void TreeHierarchy::processDrops() {
        for(const auto& drop: m_pendingDrops) {
            if(drop.action == DropAction::MakeChild) {
                if(m_makeChildCallback)
                    m_makeChildCallback(drop.source, drop.target);
            }
            else if(m_reorderCallback)
                m_reorderCallback(drop.source, drop.target);
        }
        m_pendingDrops.clear();
    }
}
