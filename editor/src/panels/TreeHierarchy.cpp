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

#include <algorithm>

#include <robot2D/imgui/Api.hpp>
#include <robot2D/Util/Logger.hpp>
#include <editor/panels/TreeHierarchy.hpp>
#include "robot2D/imgui/Util.hpp"
#include "imgui/ImGui.hpp"

namespace editor {


    void drawImage(robot2D::Texture& texture, robot2D::vec2f size, robot2D::Color backgroundColor) {

//        ImGuiWindow* window = ImGui::GetCurrentWindow();
//        const ImVec2 padding = g.Style.FramePadding;
//        const ImRect bb(window -> DC.CursorPos, window -> DC.CursorPos + image_size + padding * 2.0f);
//        ItemSize(bb);
//        if (!ItemAdd(bb, id))
//            return false;
//
//        bool hovered, held;
//        bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);
//
//        // Render
//        const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
//        RenderNavHighlight(bb, id);
//        RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, g.Style.FrameRounding));
//        if (bg_col.w > 0.0f)
//            window->DrawList->AddRectFilled(bb.Min + padding, bb.Max - padding, GetColorU32(bg_col));
//        window->DrawList->AddImage(texture_id, bb.Min + padding, bb.Max - padding, uv0, uv1, GetColorU32(tint_col));
    }


    void drawTexture(robot2D::vec2f size, robot2D::Texture* texture,
                                    robot2D::Color tintColor) {
        auto imID = ImGui::convertTextureHandle(texture -> getID());
        ImGui::ImageButton(imID, size, {0,0}, {1, 1}, -1, ImVec4(1, 1, 1, 0),
                           ImVec4(1, 1, 1, 1));
    };



    bool TreeHierarchy::deleteItem(ITreeItem::Ptr treeItem) {
        auto found = std::find_if(m_items.begin(), m_items.end(),
                                  [&treeItem](ITreeItem::Ptr item) {
                                      return item == treeItem;
                                  });
        m_deletePendingItems.emplace_back(*found);
        m_multiSelection.setQueueDeletion();
        return true;
    }


    bool TreeHierarchy::addOnSelectCallback(ItemCallback&& callback) {
        if(callback)
            m_selectCallback = std::move(callback);
        else
            return false;
        return true;
    }

    bool TreeHierarchy::addOnCallback(ItemCallback&& callback) {
        if(callback)
            m_callback = std::move(callback);
        else
            return false;
        return true;
    }

    bool TreeHierarchy::addOnReorderCallback(ReorderItemCallback&& callback) {
        if(callback)
            m_reorderCallback = std::move(callback);
        else
            return false;
        return true;
    }

    bool TreeHierarchy::addOnMakeChildCallback(ReorderItemCallback&& callback) {
        if(callback)
            m_makeAsChildCallback = std::move(callback);
        else
            return false;
        return true;
    }

    bool TreeHierarchy::addOnStopBeChildCallback(ReorderItemCallback&& callback) {
        if(callback)
            m_removeAsChildCallback = std::move(callback);
        else
            return false;
        return true;
    }

    bool TreeHierarchy::addMultiSelectCallback(MultiItemCallback&& callback) {
        if(callback)
            m_multiItemCallback = std::move(callback);
        else
            return false;
        m_multiSelection.setMultiItemCallback(std::move(m_multiItemCallback));
        return true;
    }

    bool TreeHierarchy::addMultiSelectRangeCallback(MultiItemRangeCallback&& callback) {
        if(callback)
            m_multiItemRangeCallback = std::move(callback);
        else
            return false;
        m_multiSelection.setMultiItemRangeCallback(std::move(m_multiItemRangeCallback));
        return true;
    }

    bool TreeHierarchy::addInsertItemCallback(TreeHierarchy::InsertItemCallback&& callback) {
        if(callback)
            m_insertItemCallback = std::move(callback);
        else
            return false;
        return true;
    }

    void TreeHierarchy::setBefore(ITreeItem::Ptr source, ITreeItem::Ptr target) {
        auto sourceIter = std::find_if(m_items.begin(), m_items.end(), [&source](ITreeItem::Ptr item) {
            return item -> m_id == source -> m_id;
        });

        auto targetIter = std::find_if(m_items.begin(), m_items.end(), [&target](ITreeItem::Ptr item) {
            return item -> m_id == target -> m_id;
        });

        auto sourceOldDistance = std::distance(m_items.begin(), sourceIter);
        auto targetOldDistance = std::distance(m_items.begin(), targetIter);

        if(sourceOldDistance > targetOldDistance) {
            /// insert before remove last
            m_insertItems.push_back(std::make_tuple(targetIter, source, ReorderDeleteType::Last));
        }
        else if (sourceOldDistance < targetOldDistance) {
            m_insertItems.push_back(std::make_tuple(targetIter, source, ReorderDeleteType::First));
        }
    }

    void TreeHierarchy::insertItem(ITreeItem::Ptr source, ITreeItem::Ptr anchor, bool first, bool isChained ) {

        if(first) {
            m_setItems.push_back(std::make_tuple(m_items.begin(), source, false, nullptr));
        }
        else {

            if(!isChained) {
                auto anchorFound = std::find_if(m_items.begin(), m_items.end(),
                                                [&anchor](ITreeItem::Ptr item) {
                    return item -> m_id == anchor -> m_id;
                });
                m_setItems.push_back(std::make_tuple(anchorFound, source, false, nullptr));
            }
            else {
                m_setItems.push_back(std::make_tuple(m_items.end(), source, true, anchor));
            }

        }

    }

    void TreeHierarchy::applyChildModification(ITreeItem::Ptr source, ITreeItem::Ptr target) {
        m_source = source;
        m_target = target;
    }

    void TreeHierarchy::render() {
        for(auto item: m_setItems) {
            auto iter = std::get<0>(item);
            if(iter == m_items.begin()) {
                auto insertIter = m_items.insert(iter, std::get<1>(item));
                if(insertIter != m_items.end() && m_insertItemCallback)
                    m_insertItemCallback(*insertIter);
            }
            else {
                bool isChained = std::get<2>(item);
                if(!isChained) {
                    auto insertIter = m_items.insert(std::next(iter), std::get<1>(item));
                    if(insertIter != m_items.end() && m_insertItemCallback)
                        m_insertItemCallback(*insertIter);
                }
                else {
                    auto anchor = std::get<3>(item);
                    auto prevFound = std::find_if(m_items.begin(), m_items.end(),
                                                  [&anchor](auto item) {
                        return item -> m_id == anchor -> m_id;
                    });
                    auto insertIter = m_items.insert(std::next(prevFound), std::get<1>(item));
                    if(insertIter != m_items.end() && m_insertItemCallback)
                        m_insertItemCallback(*insertIter);
                }
            }
        }
        m_setItems.clear();

        for(auto item: m_insertItems) {
            auto source = std::get<1>(item);
            m_items.insert(std::get<0>(item), source);

            if(std::get<2>(item) == ReorderDeleteType::Last) {
                auto found = util::find_last_if(m_items.begin(), m_items.end(),
                                                [&source](ITreeItem::Ptr item) {
                    return item -> m_id == source->m_id;
                });
                m_items.erase(found);
            }
            else if(std::get<2>(item) == ReorderDeleteType::First) {
                auto found = util::find_first_if(m_items.begin(), m_items.end(),
                                                 [&source](ITreeItem::Ptr item) {
                    return item -> m_id == source -> m_id;
                });
                m_items.erase(found);
            }
        }
        m_insertItems.clear();

        for(auto delItem: m_deletePendingItems) {
            m_items.erase(std::remove_if(m_items.begin(), m_items.end(), [this, &delItem](ITreeItem::Ptr item) {
                bool eq = item == delItem;
                delItem -> isQueryDeletion = false;
                if(eq && !delItem -> isChild())
                    m_multiSelection.updateItem(item, false);
                return eq;
            }), m_items.end());
        }
        m_deletePendingItems.clear();

        for(auto item: m_additemsBuffer) {
            m_items.emplace_back(item);
            constexpr bool needAdd = true;
            m_multiSelection.updateItem(item, needAdd);
        }
        m_additemsBuffer.clear();


        /// has childModification
        if(m_source && m_target) {
            m_source -> m_parent -> update();
            m_target -> addChild(m_source);
            m_source = nullptr;
            m_target = nullptr;
        }

        renderTree();
    }

    void TreeHierarchy::renderTree() {
        static ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_None | ImGuiMultiSelectFlags_ClearOnEscape;

        if(ImGui::TreeNodeEx(m_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

            ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags);
            m_multiSelection.applyRequests(ms_io, m_items);

            const bool wantDelete = m_multiSelection.hasQueryDeletion() ||
                    ( !m_multiSelection.empty() && ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete));
            ITreeItem::Ptr item_curr_idx_to_focus = nullptr;

            if(wantDelete)
                item_curr_idx_to_focus = m_multiSelection.processDeletionPreLoop(ms_io, m_items);

            for(auto item: m_items) {
                ImGuiTreeNodeFlags node_flags = m_tree_base_flags;

                if(item -> hasChildrens()) {
                    node_flags |=  ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
                }
                else {
                    node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf ;
                }

                if(item -> isQueryDeletion || !item -> m_name)
                    continue;

                auto hasItem = m_multiSelection.hasItem(item);
                if(hasItem)
                    node_flags |= ImGuiTreeNodeFlags_Selected;

                if(item -> m_iconTexture) {
                    drawTexture({20, 20}, item -> m_iconTexture, item -> m_tintColor);
                    ImGui::SameLine();
                }

                auto uuid = item -> m_id;
                ImGui::PushID(static_cast<void*>(&uuid));

                ImGui::SetNextItemSelectionUserData(static_cast<ImGuiSelectionUserData>(uuid));
                bool node_open = false;

                const char* itemName = item -> m_name -> c_str();
                node_open = ImGui::TreeNodeEx(itemName, node_flags);


                if (item_curr_idx_to_focus == item)
                    ImGui::SetKeyboardFocusHere(-1);


                if(ImGui::BeginDragDropSource( )) {
                    ImGui::SetDragDropPayload(m_playloadIdentifier.c_str(),
                                              &item -> m_id,
                                              sizeof(item -> m_id));
                    ImGui::Text("%s", item -> m_name  -> c_str());
                    ImGui::EndDragDropSource();
                }

                if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)
                        && ImGui::IsItemFocused() && ImGui::IsWindowHovered()) {
                    m_selectCallback(item);
                }

                if(!ImGui::IsKeyDown(static_cast<ImGuiKey>(robot2D::key2Int(m_shortCutKey)))) {
                    if(ImGui::BeginDragDropTarget()) {
                        auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                        if(payload) {
                            if(payload -> IsDataType(m_playloadIdentifier.c_str())) {
                                UUID id = *static_cast<UUID*>(payload -> Data);
#ifdef _WIN32
                                RB_EDITOR_INFO("Not Child: Got item with ID = {0}", id);
#else
                                // TODO(a.raag) fmt don't support UUID Correctly
#endif
                                auto found = findByID(id);
                                if(!found -> isChild() )
                                    m_reorderCallback( found, item);
                                else
                                    m_removeAsChildCallback(found, item);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }
                else if(ImGui::IsKeyDown(static_cast<ImGuiKey>(robot2D::key2Int(m_shortCutKey)))) {
                    if(ImGui::BeginDragDropTarget()) {
                        auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                        if(payload) {
                            if(payload -> IsDataType(m_playloadIdentifier.c_str())) {
                                UUID id = *static_cast<UUID*>(payload -> Data);
#ifdef _WIN32
                                RB_EDITOR_INFO("Not Child: want as make as child item with ID = {0}", id);
#else
                                // TODO(a.raag) fmt don't support UUID Correctly
#endif
                                m_makeAsChildCallback(findByID(id), item);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }


                if(node_open) {
                    if(item -> hasChildrens())
                        renderTreeChildren(item);

                    m_callback(item);

                    ImGui::TreePop();
                }

                ImGui::PopID();
            }

            ms_io = ImGui::EndMultiSelect();
            m_multiSelection.applyRequests(ms_io, m_items);

            if(wantDelete) {
                m_multiSelection.processDeletionPostLoop(ms_io, m_items, item_curr_idx_to_focus);
            }

            ImGui::TreePop();
        }
    }

    void TreeHierarchy::renderTreeChildren(ITreeItem::Ptr parent) {
        for(auto child: parent -> getChildrens()) {
            if(!child -> m_name)
                continue;

            if(child -> m_iconTexture) {
                drawTexture({20, 20}, child -> m_iconTexture, child -> m_tintColor);
                ImGui::SameLine();
            }

            ImGuiTreeNodeFlags child_node_flags = m_tree_base_flags;
            if(child -> hasChildrens()) {
                child_node_flags |=  ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
            }
            else {
                child_node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf ;
            }

            auto hasItem = m_multiSelection.hasItem(child);
            if(hasItem)
                child_node_flags |= ImGuiTreeNodeFlags_Selected;

            auto childUUID = child -> m_id;
            ImGui::PushID(static_cast<void*>(&childUUID));
            ImGui::SetNextItemSelectionUserData(static_cast<ImGuiSelectionUserData>(childUUID));

            bool childOpen = ImGui::TreeNodeEx(child -> m_name -> c_str(), child_node_flags);

            /// TODO(a.raag): recursive doNode
            if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)
               && ImGui::IsItemFocused() && ImGui::IsWindowHovered()) {
                m_selectCallback(child);
            }

            if(ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload(m_playloadIdentifier.c_str(),
                                          &child -> m_id, sizeof(child -> m_id));
                ImGui::Text(child -> m_name -> c_str());
                ImGui::EndDragDropSource();
            }


            if(ImGui::BeginDragDropTarget()) {
                auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                if(payload) {
                    if(payload -> IsDataType(m_playloadIdentifier.c_str())) {
                        UUID id = *static_cast<UUID*>(payload -> Data);
#ifdef _WIN32
                        RB_EDITOR_INFO("TreeHierachy: Want make as child item with ID = {0}", id);
#else
                        // TODO(a.raag) fmt don't support UUID Correctly
#endif
                        m_makeAsChildCallback(findByID(id), child);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if(childOpen) {
                if(child -> hasChildrens())
                    renderTreeChildren(child);

                ImGui::TreePop();
                m_callback(child);
            }
            ImGui::PopID();
        }
    }


    ITreeItem::Ptr TreeHierarchy::findByID(UUID ID) {
        auto startIter = m_items.begin();
        for(; startIter != m_items.end(); ++startIter) {
            auto start = *startIter;
            if(start -> hasChildrens()) {
                auto item = findByID(start, ID);
                if(item)
                    return item;
            }

            if(start -> m_id == ID)
                return start;
        }
        return nullptr;
    }

    ITreeItem::Ptr TreeHierarchy::findByID(ITreeItem::Ptr parent, UUID ID) {
        for(auto child: parent -> getChildrens()) {
            if(child -> hasChildrens()) {
                auto item = findByID(child, ID);
                if(item)
                    return item;
            }
            if(child -> m_id == ID)
                return child;
        }
        return nullptr;
    }

    void TreeHierarchy::clear() {
        m_items.clear();
    }

    void TreeHierarchy::setSelected(ITreeItem::Ptr item) {
        m_multiSelection.updateItem(item, true);
    }

    void TreeHierarchy::clearSelection() {
        m_multiSelection.clear();
    }



}