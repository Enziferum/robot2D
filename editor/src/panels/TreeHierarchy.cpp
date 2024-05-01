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
#include <set>

#include <robot2D/Config.hpp>
#include <robot2D/imgui/Api.hpp>
#include <robot2D/Util/Logger.hpp>
#include <robot2D/imgui/Util.hpp>
#include <imgui/ImGui.hpp>

#include <editor/panels/TreeHierarchy.hpp>

#define IMGUI_ICON_ENTITY			u8"\ue000"

namespace editor {
    TreeHierarchy::TreeHierarchy(std::string name) : m_name(std::move(name)) {
        m_tree_base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
            | ImGuiTreeNodeFlags_SpanAvailWidth;
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
        if(!callback)
            return false;
        m_multiSelection.setMultiItemCallback(std::move(m_multiItemCallback));
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

    void TreeHierarchy::restoreItem(TreeHierarchy::RestoreInfo&& restoreInfo) {

        if(restoreInfo.isFirst) {
            restoreInfo.m_sourceIterator = m_items.begin();
        }
        else {

            if(!restoreInfo.isChained) {
                auto anchorFound = std::find_if(m_items.begin(), m_items.end(),
                                                [&restoreInfo](ITreeItem::Ptr item) {
                    return item -> m_id == restoreInfo.anchor -> m_id;
                });
                restoreInfo.m_sourceIterator = anchorFound;
            }
            else {
                restoreInfo.m_sourceIterator = m_items.end();
            }
        }

        m_restoreItems.push_back(std::move(restoreInfo));

    }

    bool TreeHierarchy::deleteItem(ITreeItem::Ptr treeItem) {
        auto found = std::find_if(m_items.begin(), m_items.end(),
            [&treeItem](ITreeItem::Ptr item) {
                return item == treeItem;
            });
        m_deletePendingItems.emplace_back(*found);
        //m_multiSelection.setQueueDeletion();
        return true;
    }


    void TreeHierarchy::applyChildModification(ITreeItem::Ptr source, ITreeItem::Ptr target) {
        m_source = source;
        m_target = target;
    }

    ITreeItem::Ptr TreeHierarchy::findByID(UUID ID) const{
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

    ITreeItem::Ptr TreeHierarchy::findByID(ITreeItem::Ptr parent, UUID ID) const {
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
        m_multiSelection.addItem(item);
    }

    void TreeHierarchy::clearSelection() {
        m_multiSelection.clearAll();
    }

    void TreeHierarchy::update() {
        for (auto& item : m_restoreItems) {
            auto& iter = item.m_sourceIterator;
            if (item.isFirst) {
                m_items.insert(m_items.begin(), item.target);
            }
            else {
                bool isChained = item.isChained;
                if (!isChained) {
                    auto insertIter = m_items.insert(std::next(iter), item.target);
                }
                else {
                    auto anchor = item.anchor;
                    auto prevFound = std::find_if(m_items.begin(), m_items.end(),
                        [&anchor](auto item) {
                            return item -> m_id == anchor -> m_id;
                        });
                    auto insertIter = m_items.insert(std::next(prevFound), item.target);
                }
            }
        }
        m_restoreItems.clear();

        for (auto& item : m_insertItems) {
            auto source = std::get<1>(item);
            m_items.insert(std::get<0>(item), source);

            if (std::get<2>(item) == ReorderDeleteType::Last) {
                auto found = util::find_last_if(m_items.begin(), m_items.end(),
                    [&source](ITreeItem::Ptr item) {
                        return item -> m_id == source -> m_id;
                    });
                m_items.erase(found);
            }
            else if (std::get<2>(item) == ReorderDeleteType::First) {
                auto found = util::find_first_if(m_items.begin(), m_items.end(),
                    [&source](ITreeItem::Ptr item) {
                        return item -> m_id == source -> m_id;
                    });
                m_items.erase(found);
            }
        }
        m_insertItems.clear();

        for (auto& delItem : m_deletePendingItems) {
            m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
                                                [this, &delItem](ITreeItem::Ptr item) {
                bool eq = item == delItem;
                delItem -> isQueryDeletion = false;
                if (eq && !delItem->isChild())
                    m_multiSelection.removeItem(item);
                return eq;
                }), m_items.end());
        }
        m_deletePendingItems.clear();

        for (auto& item : m_additemsBuffer) {
            m_items.emplace_back(item);
            m_multiSelection.addItem(item);
        }
        m_additemsBuffer.clear();


        /// has childModification
        if (m_source && m_target) {
            m_source -> m_parent-> update();
            m_target -> addChild(m_source);
            m_source = nullptr;
            m_target = nullptr;
        }

    }

    void TreeHierarchy::render() {
        update();
        renderTree();
    }


    void TreeHierarchy::renderTree() {
        if (ImGui::TreeNodeEx(m_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            m_multiSelection.preUpdate(m_items);

            for (auto& item : m_items) {
                ImGuiTreeNodeFlags node_flags = m_tree_base_flags | ImGuiTreeNodeFlags_FramePadding
                                                                            | ImGuiTreeNodeFlags_Framed ;

                if (item -> hasChildrens())
                    node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
                else
                    node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;


                if (item -> isQueryDeletion || !item -> m_name)
                    continue;

                auto hasItem = m_multiSelection.hasItem(item);
                if (hasItem)
                    node_flags |= ImGuiTreeNodeFlags_Selected;

                bool node_open = false;

                {
                    std::string treeNodeText;
                    treeNodeText += IMGUI_ICON_ENTITY + std::string("  ") + *item -> m_name;

                    robot2D::ScopedStyleVarVec2 var(ImGuiStyleVar_FramePadding, { 2, 2 });
                    if(!hasItem)
                        ImGui::PushStyleColor(ImGuiCol_Header, { 0.0, 0.0, 0.0, 0.0 });
                    else
                        ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);

                    node_open = ImGui::TreeNodeEx(treeNodeText.c_str(), node_flags);

                    ImGui::PopStyleColor();
                }


                m_multiSelection.update();

                if(ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    m_multiSelection.markSelected(item);
                    if(m_multiSelection.isSingleSelect())
                        m_selectCallback(item);
                }

                imgui_DragDropSource() {
                    ImGui::SetDragDropPayload(m_playloadIdentifier.c_str(), &item->m_id, sizeof(item->m_id));
                    imgui_Text(item -> m_name -> c_str());
                }

                bool altDown = ImGui::GetIO().KeyAlt;

                if (!ImGui::IsKeyDown(static_cast<ImGuiKey>(robot2D::key2Int(m_shortCutKey)))) {
                    robot2D::DragDropTarget dragDropTarget{m_playloadIdentifier};
                    if(auto uuid = dragDropTarget.unpackPayload<UUID>()) {
#ifdef ROBOT2D_WINDOWS
                       // RB_EDITOR_INFO( "Not Child: Got item with ID = {0}", *uuid );
#else
                        // TODO(a.raag) fmt don't support UUID Correctly
#endif
                        auto found = findByID(*uuid);
                        if (!found -> isChild())
                            m_reorderCallback(found, item);
                        else
                            m_removeAsChildCallback(found, item);
                    }
                }
                else if (ImGui::IsKeyDown(static_cast<ImGuiKey>(robot2D::key2Int(m_shortCutKey)))) {
                    robot2D::DragDropTarget dragDropTarget{m_playloadIdentifier};
                    if(auto uuid = dragDropTarget.unpackPayload<UUID>()) {
#ifdef ROBOT2D_WINDOWS
                     //   RB_EDITOR_INFO("Not Child: want as make as child item with ID = {0}", *uuid);
#else
                        // TODO(a.raag) fmt don't support UUID Correctly
#endif
                        m_makeAsChildCallback(findByID(*uuid), item);
                    }
                }

                if (node_open) {
                    if (item -> hasChildrens())
                        renderTreeChildren(item);

                    m_callback(item);
                    ImGui::TreePop();
                }
            }

            m_multiSelection.postUpdate();

            ImGui::TreePop();
        }
    }

    void TreeHierarchy::renderTreeChildren(ITreeItem::Ptr parent) {
        for (auto child : parent -> getChildrens()) {
            if (!child -> m_name)
                continue;

            ImGuiTreeNodeFlags child_node_flags = m_tree_base_flags | ImGuiTreeNodeFlags_FramePadding
                                                                                    | ImGuiTreeNodeFlags_Framed;
            if (child -> hasChildrens())
                child_node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
            else
                child_node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;

            auto hasItem = m_multiSelection.hasItem(child);
            if (hasItem)
                child_node_flags |= ImGuiTreeNodeFlags_Selected;

            bool childOpen = false;

            {
                std::string treeNodeText;
                treeNodeText += IMGUI_ICON_ENTITY + std::string("  ") + *child -> m_name;

                robot2D::ScopedStyleVarVec2 var(ImGuiStyleVar_FramePadding, { 2, 2 });
                if(!hasItem)
                    ImGui::PushStyleColor(ImGuiCol_Header, { 0.0, 0.0, 0.0, 0.0 });
                else
                    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
                childOpen = ImGui::TreeNodeEx(treeNodeText.c_str(), child_node_flags);
                ImGui::PopStyleColor();
            }

            /// TODO(a.raag): recursive doNode
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)
                && ImGui::IsItemFocused() && ImGui::IsWindowHovered()) {
                m_selectCallback(child);
            }

            imgui_DragDropSource() {
                ImGui::SetDragDropPayload(m_playloadIdentifier.c_str(), &child->m_id, sizeof(child -> m_id));
                ImGui::Text("%s", child->m_name->c_str());
            }

            {
                robot2D::DragDropTarget dragDropTarget{m_playloadIdentifier};
                if(auto uuid = dragDropTarget.unpackPayload<UUID>()) {
#ifdef ROBOT2D_WINDOWS
                  //  RB_EDITOR_INFO("TreeHierachy: Want make as child item with ID = {0}", *uuid);
#else
                    // TODO(a.raag) fmt don't support UUID Correctly
#endif
                    m_makeAsChildCallback(findByID(*uuid), child);
                }
            }

            if (childOpen) {
                if (child -> hasChildrens())
                    renderTreeChildren(child);

                ImGui::TreePop();
                m_callback(child);
            }
        }
    }

    const std::size_t TreeHierarchy::getItemsValue() const {
        std::size_t size = { 0 };
        for(auto& item: m_items)
            size += item -> getChildValue(size);
        size += m_items.size();
        return size;
    }


}