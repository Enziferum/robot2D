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

    namespace {
        class TreeSelection {
        public:
            TreeSelection() = default;
            TreeSelection(const TreeSelection& other) = delete;
            TreeSelection& operator=(const TreeSelection& other) = delete;
            TreeSelection(TreeSelection&& other) = delete;
            TreeSelection& operator=(TreeSelection&& other) = delete;
            ~TreeSelection() = default;


            bool hasItem(ITreeItem::Ptr item) const {
                return std::find_if(m_selectedItems.begin(), m_selectedItems.end(), [&item](ITreeItem::Ptr obj) {
                    return *item == *obj;
                }) != m_selectedItems.end();
            }

            void preUpdate(std::list<ITreeItem::Ptr>& items);
            void update();
            void postUpdate();
            /// \brief Using only due to Hierarchy process, to outside update use AddItem() / removeItem()
            void markSelected(ITreeItem::Ptr item);

            void addItem(ITreeItem::Ptr item) {
                m_selectedItems.insert(item);
            }

            void removeItem();
            void clearAll();

            bool isSingleSelect() const {
                return m_currentState == State::SingleSelect;
            }
        private:
            enum class State {
                Clear,
                SingleSelect,
                MultiSelect,
                MultiAllSelect,
                RangeSelect
            };

            State m_lastState = State::SingleSelect;
            State m_currentState = State::SingleSelect;


            std::set<ITreeItem::Ptr> m_preSelectedItems;
            std::set<ITreeItem::Ptr> m_selectedItems;

            bool m_ControlDown{ false };
            bool m_ShiftDown{ false };

            const int m_rangeMaxItems = 2;
        };


        void TreeSelection::preUpdate(std::list<ITreeItem::Ptr>& items) {

            switch(m_currentState) {
                case State::Clear:
                    m_selectedItems.clear();
                    break;
                case State::SingleSelect: {
                    if(!m_preSelectedItems.empty())
                        m_selectedItems.clear();
                    for(auto& item: m_preSelectedItems)
                        m_selectedItems.insert(item);
                    break;
                }
                case State::MultiSelect: {
                    for(auto& item: m_preSelectedItems)
                        m_selectedItems.insert(item);
                    /// TODO(a.raag): multiselect callback
                    break;
                }
                case State::MultiAllSelect: {
                    for(auto& item: items)
                        m_selectedItems.insert(item);

                    /// TODO(a.raag): multiallselect callback
                    break;
                }
                case State::RangeSelect: {
                    if(m_selectedItems.empty() && !m_preSelectedItems.empty())
                    {
                        m_selectedItems.insert(*m_preSelectedItems.begin());
                        break;
                    }
                    else if(m_preSelectedItems.size() == 1 && m_selectedItems.size() == 1) {
                        std::vector<ITreeItem::Ptr> rangeItems;
                        rangeItems.push_back(*m_selectedItems.begin());
                        rangeItems.push_back(*m_preSelectedItems.begin());

                        auto firstFound = std::find_if(items.begin(), items.end(),
                                                       [&rangeItems](const ITreeItem::Ptr& ptr) {
                                                           return *ptr == *rangeItems[0];
                                                       });

                        auto lastFound = std::find_if(items.begin(), items.end(),
                                                      [&rangeItems](const ITreeItem::Ptr& ptr) {
                                                          return *ptr == *rangeItems[1];
                                                      });

                        if(firstFound != items.end() && lastFound != items.end()) {
                            for(; firstFound != lastFound; ++firstFound)
                                m_selectedItems.insert(*firstFound);
                            m_selectedItems.insert(*lastFound);


                            /// TODO(a.raag): rangeSelect callback
                        }

                    }

                    break;
                }
            }

            m_preSelectedItems.clear();
        }


        void TreeSelection::update() {
            m_ControlDown = ImGui::GetIO().KeyCtrl;
            m_ShiftDown = ImGui::GetIO().KeyShift;

            if(m_ControlDown && !m_ShiftDown && ImGui::IsKeyDown(ImGuiKey_A))
                m_currentState = State::MultiAllSelect;
            else if(!m_ControlDown && m_ShiftDown)
                m_currentState = State::RangeSelect;
            else if(m_ControlDown && !m_ShiftDown)
                m_currentState = State::MultiSelect;
            else if(!m_ControlDown && !m_ShiftDown)
                m_currentState = State::SingleSelect;
        }

        void TreeSelection::postUpdate() {
            bool needClear = false;
            if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()
               && !m_selectedItems.empty() && !(m_currentState == State::MultiSelect
                                                || m_currentState == State::RangeSelect))
                needClear = true;

            if(ImGui::IsKeyPressed(ImGuiKey_Escape) || needClear)
                m_currentState = State::Clear;
        }

        void TreeSelection::markSelected(ITreeItem::Ptr item) {

            switch(m_currentState) {
                case State::Clear:
                case State::MultiAllSelect:
                    break;
                case State::RangeSelect: {

                    if(m_preSelectedItems.size() < m_rangeMaxItems)
                        m_preSelectedItems.insert(item);
                    break;
                }
                case State::MultiSelect: {
                    m_preSelectedItems.insert(item);
                    break;
                }
                case State::SingleSelect: {
                    m_preSelectedItems.insert(item);
                    break;
                }
            }
        }

        void TreeSelection::clearAll() {
            m_preSelectedItems.clear();
            m_selectedItems.clear();
        }

        static TreeSelection exp_multiSelection;

    }


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

    bool TreeHierarchy::deleteItem(ITreeItem::Ptr treeItem) {
        auto found = std::find_if(m_items.begin(), m_items.end(),
            [&treeItem](ITreeItem::Ptr item) {
                return item == treeItem;
            });
        m_deletePendingItems.emplace_back(*found);
        m_multiSelection.setQueueDeletion();
        return true;
    }


    void TreeHierarchy::applyChildModification(ITreeItem::Ptr source, ITreeItem::Ptr target) {
        m_source = source;
        m_target = target;
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
#ifdef RB2D_OLD_MULTISELECTION
        m_multiSelection.updateItem(item, true);
#else
        exp_multiSelection.addItem(item);
#endif
    }

    void TreeHierarchy::clearSelection() {
#ifdef RB2D_OLD_MULTISELECTION
        m_multiSelection.clear();
#else
        exp_multiSelection.clearAll();
#endif
    }

    void TreeHierarchy::update() {
        for (auto item : m_setItems) {
            auto iter = std::get<0>(item);
            if (iter == m_items.begin()) {
                auto insertIter = m_items.insert(iter, std::get<1>(item));
                if (insertIter != m_items.end() && m_insertItemCallback)
                    m_insertItemCallback(*insertIter);
            }
            else {
                bool isChained = std::get<2>(item);
                if (!isChained) {
                    auto insertIter = m_items.insert(std::next(iter), std::get<1>(item));
                    if (insertIter != m_items.end() && m_insertItemCallback)
                        m_insertItemCallback(*insertIter);
                }
                else {
                    auto anchor = std::get<3>(item);
                    auto prevFound = std::find_if(m_items.begin(), m_items.end(),
                        [&anchor](auto item) {
                            return item->m_id == anchor->m_id;
                        });
                    auto insertIter = m_items.insert(std::next(prevFound), std::get<1>(item));
                    if (insertIter != m_items.end() && m_insertItemCallback)
                        m_insertItemCallback(*insertIter);
                }
            }
        }
        m_setItems.clear();

        for (auto item : m_insertItems) {
            auto source = std::get<1>(item);
            m_items.insert(std::get<0>(item), source);

            if (std::get<2>(item) == ReorderDeleteType::Last) {
                auto found = util::find_last_if(m_items.begin(), m_items.end(),
                    [&source](ITreeItem::Ptr item) {
                        return item->m_id == source->m_id;
                    });
                m_items.erase(found);
            }
            else if (std::get<2>(item) == ReorderDeleteType::First) {
                auto found = util::find_first_if(m_items.begin(), m_items.end(),
                    [&source](ITreeItem::Ptr item) {
                        return item->m_id == source->m_id;
                    });
                m_items.erase(found);
            }
        }
        m_insertItems.clear();

        for (auto delItem : m_deletePendingItems) {
            m_items.erase(std::remove_if(m_items.begin(), m_items.end(), [this, &delItem](ITreeItem::Ptr item) {
                bool eq = item == delItem;
                delItem->isQueryDeletion = false;
                if (eq && !delItem->isChild())
                    m_multiSelection.updateItem(item, false);
                return eq;
                }), m_items.end());
        }
        m_deletePendingItems.clear();

        for (auto item : m_additemsBuffer) {
            m_items.emplace_back(item);
            constexpr bool needAdd = true;
            m_multiSelection.updateItem(item, needAdd);
        }
        m_additemsBuffer.clear();


        /// has childModification
        if (m_source && m_target) {
            m_source->m_parent->update();
            m_target->addChild(m_source);
            m_source = nullptr;
            m_target = nullptr;
        }

    }

    void TreeHierarchy::render() {
        update();
        renderTree();
    }


    template<typename T>
    class DragDropSource {
    public:
        DragDropSource() = delete;
        DragDropSource(const std::string& ID, const T& value) {
            if(ImGui::BeginDragDropSource()) {
                m_beginSource = true;
                ImGui::SetDragDropPayload(ID.c_str(), &value, sizeof(value));
            }
        }
        ~DragDropSource() {
            if(m_beginSource)
                ImGui::EndDragDropSource();
        }

    private:
        bool m_beginSource { false };
    };

    void TreeHierarchy::renderTree() {
        static ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_None | ImGuiMultiSelectFlags_ClearOnEscape;

        if (ImGui::TreeNodeEx(m_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            exp_multiSelection.preUpdate(m_items);
#ifdef RB2D_OLD_MULTISELECTION
        ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags);
        m_multiSelection.applyRequests(ms_io, m_items);

        const bool wantDelete = m_multiSelection.hasQueryDeletion() ||
        (!m_multiSelection.empty() && ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete));
#endif
        ITreeItem::Ptr item_curr_idx_to_focus = nullptr;
#ifdef RB2D_OLD_MULTISELECTION
        if (wantDelete)
            item_curr_idx_to_focus = m_multiSelection.processDeletionPreLoop(ms_io, m_items);
#endif
            for (auto item : m_items) {
                ImGuiTreeNodeFlags node_flags = m_tree_base_flags | ImGuiTreeNodeFlags_FramePadding
                                                                            | ImGuiTreeNodeFlags_Framed ;

                if (item -> hasChildrens())
                    node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
                else
                    node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;


                if (item->isQueryDeletion || !item->m_name)
                    continue;

                auto hasItem = exp_multiSelection.hasItem(item);
                if (hasItem)
                    node_flags |= ImGuiTreeNodeFlags_Selected;

                auto uuid = item -> m_id;
                bool node_open = false;
                const char* itemName = item->m_name->c_str();
                std::string treeNodeText;
                treeNodeText += IMGUI_ICON_ENTITY + std::string("  ") + std::string(itemName);

                ImGui::PushID(static_cast<void*>(&uuid));
                ImGui::SetNextItemSelectionUserData(static_cast<ImGuiSelectionUserData>(uuid));
                {
                    robot2D::ScopedStyleVarVec2 var(ImGuiStyleVar_FramePadding, { 2, 2 });
                    if(!hasItem)
                        ImGui::PushStyleColor(ImGuiCol_Header, { 0.0, 0.0, 0.0, 0.0 });
                    else
                        ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
                    /// TODO(a.raag): check correctness of using id as ptr_id
                    node_open = ImGui::TreeNodeEx(static_cast<void*>(&uuid), node_flags, "%s", treeNodeText.c_str());

                    ImGui::PopStyleColor();
                }


                exp_multiSelection.update();

                if(ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    exp_multiSelection.markSelected(item);
                    if(exp_multiSelection.isSingleSelect())
                        m_selectCallback(item);
                }


                if (item_curr_idx_to_focus == item)
                    ImGui::SetKeyboardFocusHere(-1);



                {
                    DragDropSource dragDropSource{m_playloadIdentifier, item -> m_id};
                }
                if (ImGui::BeginDragDropSource()) {
                    ImGui::SetDragDropPayload(m_playloadIdentifier.c_str(),
                        &item->m_id,
                        sizeof(item->m_id));
                    imgui_Text(item -> m_name);
                    ImGui::EndDragDropSource();
                }

                if (!ImGui::IsKeyDown(static_cast<ImGuiKey>(robot2D::key2Int(m_shortCutKey)))) {
                    if (ImGui::BeginDragDropTarget()) {
                        auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                        if (payload && payload->IsDataType(m_playloadIdentifier.c_str())) {
                            UUID id = *static_cast<UUID*>(payload->Data);
#ifdef ROBOT2D_WINDOWS
                            RB_EDITOR_INFO("Not Child: Got item with ID = {0}", id);
#else
                            // TODO(a.raag) fmt don't support UUID Correctly
#endif
                            auto found = findByID(id);
                            if (!found -> isChild())
                                m_reorderCallback(found, item);
                            else
                                m_removeAsChildCallback(found, item);
                        }
                        ImGui::EndDragDropTarget();
                    }
                }
                else if (ImGui::IsKeyDown(static_cast<ImGuiKey>(robot2D::key2Int(m_shortCutKey)))) {
                    if (ImGui::BeginDragDropTarget()) {
                        auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                        if (payload && payload->IsDataType(m_playloadIdentifier.c_str())) {
                            UUID id = *static_cast<UUID*>(payload->Data);
#ifdef ROBOT2D_WINDOWS
                            RB_EDITOR_INFO("Not Child: want as make as child item with ID = {0}", id);
#else
                            // TODO(a.raag) fmt don't support UUID Correctly
#endif
                            m_makeAsChildCallback(findByID(id), item);
                        }
                        ImGui::EndDragDropTarget();
                    }
                }


                if (node_open) {
                    if (item -> hasChildrens())
                        renderTreeChildren(item);

                    m_callback(item);
                    ImGui::TreePop();
                }

                ImGui::PopID();
            }
#ifdef RB2D_OLD_MULTISELECTION
            ms_io = ImGui::EndMultiSelect();
            m_multiSelection.applyRequests(ms_io, m_items);

            if (wantDelete) {
                m_multiSelection.processDeletionPostLoop(ms_io, m_items, item_curr_idx_to_focus);
            }
#endif
            exp_multiSelection.postUpdate();

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

            auto hasItem = exp_multiSelection.hasItem(child);
            if (hasItem)
                child_node_flags |= ImGuiTreeNodeFlags_Selected;

            auto childUUID = child -> m_id;
            bool childOpen = false;

            ImGui::PushID(static_cast<void*>(&childUUID));
            ImGui::SetNextItemSelectionUserData(static_cast<ImGuiSelectionUserData>(childUUID));

            std::string treeNodeText;
            treeNodeText += IMGUI_ICON_ENTITY + std::string("  ") + *child -> m_name;

            {
                robot2D::ScopedStyleVarVec2 var(ImGuiStyleVar_FramePadding, { 2, 2 });
                if(!hasItem)
                    ImGui::PushStyleColor(ImGuiCol_Header, { 0.0, 0.0, 0.0, 0.0 });
                else
                    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
                /// TODO(a.raag): check correctness of using id as ptr_id
                childOpen = ImGui::TreeNodeEx(static_cast<void*>(&childUUID), child_node_flags, "%s", treeNodeText.c_str());
                ImGui::PopStyleColor();
            }

            /// TODO(a.raag): recursive doNode
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)
                && ImGui::IsItemFocused() && ImGui::IsWindowHovered()) {
                m_selectCallback(child);
            }

            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload(m_playloadIdentifier.c_str(),
                    &child->m_id, sizeof(child->m_id));
                ImGui::Text("%s", child->m_name->c_str());
                ImGui::EndDragDropSource();
            }


            if (ImGui::BeginDragDropTarget()) {
                auto* payload = ImGui::AcceptDragDropPayload(m_playloadIdentifier.c_str());
                if (payload && payload -> IsDataType(m_playloadIdentifier.c_str())) {
                    UUID id = *static_cast<UUID*>(payload->Data);
#ifdef ROBOT2D_WINDOWS
                    RB_EDITOR_INFO("TreeHierachy: Want make as child item with ID = {0}", id);
#else
                    // TODO(a.raag) fmt don't support UUID Correctly
#endif
                    m_makeAsChildCallback(findByID(id), child);
                
                }
                ImGui::EndDragDropTarget();
            }

            if (childOpen) {
                if (child->hasChildrens())
                    renderTreeChildren(child);

                ImGui::TreePop();
                m_callback(child);
            }
            ImGui::PopID();
        }
    }

}