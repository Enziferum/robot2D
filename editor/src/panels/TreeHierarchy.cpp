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

#include <robot2D/Util/Logger.hpp>
#include <robot2D/imgui/Api.hpp>
#include <editor/panels/TreeHierarchy.hpp>

namespace editor {

    ITreeItem::~ITreeItem() = default;

    bool TreeHierarchy::deleteItem(ITreeItem::Ptr treeItem) {
        auto found = std::find_if(m_items.begin(), m_items.end(),
                                  [&treeItem](ITreeItem::Ptr item) {
                                      return item == treeItem;
                                  });
        m_deletePendingItems.emplace_back(*found);
        return true;
    }

    void TreeHierarchy::render() {
        for(auto item: m_insertItems) {
            auto source = std::get<1>(item);
            m_items.insert(std::get<0>(item), source);

            if(std::get<2>(item) == ReorderDeleteType::Last) {
                auto found = util::find_last_if(m_items.begin(), m_items.end(), [&source](ITreeItem::Ptr item) {
                    return item->m_id == source->m_id;
                });
                m_items.erase(found);
            }
            else if(std::get<2>(item) == ReorderDeleteType::First) {
                auto found = util::find_first_if(m_items.begin(), m_items.end(), [&source](ITreeItem::Ptr item) {
                    return item->m_id == source->m_id;
                });
                m_items.erase(found);
            }
        }
        m_insertItems.clear();

        for(auto delItem: m_deletePendingItems) {
            m_items.erase(std::remove_if(m_items.begin(), m_items.end(), [&delItem](ITreeItem::Ptr item) {
                return item == delItem;
            }), m_items.end());
        }
        m_deletePendingItems.clear();

        for(auto item: m_additemsBuffer)
            m_items.emplace_back(item);
        m_additemsBuffer.clear();

        if(ImGui::TreeNodeEx(m_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

            for(auto item: m_items) {
                ImGuiTreeNodeFlags node_flags = m_tree_base_flags;

                if(item -> hasChildrens()) {
                    node_flags |= ((m_selectedID == item -> getID()
                            && m_childSelectedID == NO_INDEX) ? ImGuiTreeNodeFlags_Selected : 0);
                    node_flags |=  ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
                    bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)item -> m_id, node_flags,
                                                       item -> m_name -> c_str());

                    auto& childrens = item -> getChildrens();

                    if(ImGui::IsItemClicked()) {
                        m_selectedID = item -> m_id;
                        m_childSelectedID = NO_INDEX;
                        m_selectCallback(item);
                    }

                    m_callback(item);

                    if(node_open) {
                        if(ImGui::BeginDragDropSource()) {
                            ImGui::SetDragDropPayload(m_playloadIdentifier.c_str(), &item -> m_id,
                                                                                    sizeof(item -> m_id));
                            ImGui::Text(item -> m_name -> c_str());
                            ImGui::EndDragDropSource();
                        }

                        if(ImGui::BeginDragDropTarget()) {
                            auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                            if(payload) {
                                if(payload -> IsDataType(m_playloadIdentifier.c_str())) {
                                    UUID id = *static_cast<UUID*>(payload -> Data);
                                    RB_EDITOR_INFO("Treehierachy: got item with ID = {0}", id);
                                    m_reorderCallback(findByID(id), item);
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }

                        for(auto child: childrens) {
                            ImGuiTreeNodeFlags child_node_flags = m_tree_base_flags;
                            child_node_flags |= ((m_childSelectedID == child -> m_child_id)
                                                          ? ImGuiTreeNodeFlags_Selected : 0);
                            child_node_flags |= ImGuiTreeNodeFlags_Leaf;
                            ImGui::TreeNodeEx((void*)(intptr_t)item -> m_child_id, child_node_flags,
                                              child -> m_name -> c_str());

                            /// recursive doNode
                            if(ImGui::IsItemClicked()) {
                                m_selectedID = NO_INDEX;
                                m_childSelectedID = child -> m_child_id;
                                m_selectCallback(child);
                            }

                            m_callback(child);

                            if(ImGui::BeginDragDropSource()) {
                                ImGui::SetDragDropPayload(m_playloadIdentifier.c_str(),
                                                          &child -> m_child_id, sizeof(child -> m_child_id));
                                ImGui::Text(item -> m_name -> c_str());
                                ImGui::EndDragDropSource();
                            }


                            if(ImGui::BeginDragDropTarget()) {
                                auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                                if(payload) {
                                    if(payload -> IsDataType(m_playloadIdentifier.c_str())) {
                                        UUID id = *static_cast<UUID*>(payload -> Data);
                                        RB_EDITOR_INFO("TreeHierachy: Want make as child item with ID = {0}", id);
                                        m_makeAsChildCallback(findByID(id), item);
                                    }
                                }
                                ImGui::EndDragDropTarget();
                            }


                            ImGui::TreePop();
                        }
                    }

                    if(node_open)
                        ImGui::TreePop();
                }
                else {
                    node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                    node_flags |= ((m_selectedID == item -> getID()) ? ImGuiTreeNodeFlags_Selected : 0);
                    auto ida = item -> m_id;
                    bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)ida, node_flags,
                                                       item -> m_name -> c_str());

                    if(ImGui::IsItemClicked()) {
                        m_selectedID = item -> m_id;
                        m_childSelectedID = NO_INDEX;
                        m_selectCallback(item);
                    }

                    m_callback(item);

                    if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID )) {
                        ImGui::SetDragDropPayload(m_playloadIdentifier.c_str(), &item -> m_id, sizeof(item -> m_id));
                        ImGui::Text(item -> m_name -> c_str());
                        ImGui::EndDragDropSource();
                    }

                    if(!ImGui::IsKeyDown(static_cast<ImGuiKey>(robot2D::key2Int(m_shortCutKey)))) {
                        if(ImGui::BeginDragDropTarget()) {
                            auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                            if(payload) {
                                if(payload -> IsDataType(m_playloadIdentifier.c_str())) {
                                    UUID id = *static_cast<UUID*>(payload -> Data);
                                    RB_EDITOR_INFO("Not Child: Got item with ID = {0}", id);
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
                                    RB_EDITOR_INFO("Not Child: want as make as child item with ID = {0}", id);
                                    m_makeAsChildCallback(findByID(id), item);
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                    }
                }
            }
            ImGui::TreePop();
        }

    }

    ITreeItem::Ptr TreeHierarchy::findByID(UUID ID) {
        ITreeItem::Ptr parent = nullptr;
        auto found = std::find_if(m_items.begin(), m_items.end(), [ID, &parent](ITreeItem::Ptr item) {
            for(auto child: item -> getChildrens()) {
                if(child -> m_child_id == ID) {
                    parent = item;
                    return false;
                }
            }
            return item -> m_id == ID;
        });
        if(found == m_items.end() && !parent)
            return nullptr;

        if(parent) {
            for(auto child: parent -> getChildrens()) {
                if (child -> m_child_id == ID) {
                    return child;
                }
            }
        }

        return *found;
    }

    void TreeHierarchy::clear() {
        m_items.clear();
    }

    void TreeHierarchy::setSelected(ITreeItem::Ptr item) {
        if(item -> isChild()) {
            m_selectedID = NO_INDEX;
            m_childSelectedID = item -> m_child_id;
        }
        else {
            m_selectedID = item -> getID();
            m_childSelectedID = NO_INDEX;
        }
    }

}