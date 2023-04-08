#include <algorithm>

#include <robot2D/Util/Logger.hpp>
#include <robot2D/imgui/Api.hpp>
#include "TreeHierarchy.hpp"

namespace robot2D {

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


        if(ImGui::TreeNode(m_name.c_str())) {

            for(auto item: m_items) {
                ImGuiTreeNodeFlags node_flags = m_tree_base_flags;

                if(item -> hasChildrens()) {
                    node_flags |= ((m_selectedID == item -> getID()) ? ImGuiTreeNodeFlags_Selected : 0);
                    node_flags |=  ImGuiTreeNodeFlags_SpanAvailWidth;
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
                            ImGui::SetDragDropPayload(m_playloadIdentifier.c_str(), &item -> m_id, sizeof(item -> m_id));
                            ImGui::Text(item -> m_name -> c_str());
                            ImGui::EndDragDropSource();
                        }

                        if(ImGui::BeginDragDropTarget()) {
                            auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                            if(payload) {
                                if(payload -> IsDataType(m_playloadIdentifier.c_str())) {
                                    int id = *static_cast<int*>(payload -> Data);
                                    RB_EDITOR_INFO("TreeHierachy: Got item with ID = {0}", id);
                                    m_reorderCallback(m_items[id], item);
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }

                        for(auto child: childrens) {
                            ImGuiTreeNodeFlags child_node_flags = m_tree_base_flags;
                            child_node_flags |= ((m_selectedID == child -> getID()
                                                  && m_childSelectedID == child -> m_child_id)
                                                          ? ImGuiTreeNodeFlags_Selected : 0);
                            child_node_flags |= ImGuiTreeNodeFlags_Leaf;
                            ImGui::TreeNodeEx((void*)(intptr_t)item -> m_child_id, child_node_flags,
                                              child -> m_name -> c_str());

                            /// recursive doNode
                            if(ImGui::IsItemClicked()) {
                                m_selectedID = item -> m_id;
                                m_childSelectedID = item -> m_child_id;
                                m_selectCallback(child);
                            }

                            m_callback(child);

                            if(ImGui::BeginDragDropTarget()) {
                                auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                                if(payload) {
                                    if(payload -> IsDataType(m_playloadIdentifier.c_str())) {
                                        int id = *static_cast<int*>(payload -> Data);
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
                    int a = item -> m_id;
                    bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)a, node_flags,
                                                       item -> m_name -> c_str());

                    if(ImGui::IsItemClicked()) {
                        m_selectedID = item -> m_id;
                        m_childSelectedID = NO_INDEX;
                        m_selectCallback(item);
                    }

                    m_callback(item);

                    if(ImGui::BeginDragDropSource()) {
                        ImGui::SetDragDropPayload(m_playloadIdentifier.c_str(), &item -> m_id, sizeof(item -> m_id));
                        ImGui::Text(item -> m_name -> c_str());
                        ImGui::EndDragDropSource();
                    }

                    if(!ImGui::IsKeyDown(robot2D::key2Int(m_shortCutKey))) {
                        if(ImGui::BeginDragDropTarget()) {
                            auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                            if(payload) {
                                if(payload -> IsDataType(m_playloadIdentifier.c_str())) {
                                    int id = *static_cast<int*>(payload -> Data);
                                    RB_EDITOR_INFO("TreeHierachy: Got item with ID = {0}", id);
                                    m_reorderCallback(findByID(id), item);
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                    }
                    else if(ImGui::IsKeyDown(robot2D::key2Int(m_shortCutKey))) {
                        if(ImGui::BeginDragDropTarget()) {
                            auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                            if(payload) {
                                if(payload -> IsDataType(m_playloadIdentifier.c_str())) {
                                    int id = *static_cast<int*>(payload -> Data);
                                    RB_EDITOR_INFO("TreeHierachy: want as make as child item with ID = {0}", id);
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

    ITreeItem::Ptr TreeHierarchy::findByID(int ID) {
        auto found = std::find_if(m_items.begin(), m_items.end(), [ID](ITreeItem::Ptr item) {
            return item -> m_id == ID;
        });
        if(found == m_items.end())
            return nullptr;
        return *found;
    }
}