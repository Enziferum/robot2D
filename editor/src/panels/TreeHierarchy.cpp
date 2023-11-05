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



    class MultiSelection {
    public:
        using MultiItemCallback = std::function<void(std::vector<ITreeItem::Ptr>)>;
        using MultiItemRangeCallback = std::function<void(std::vector<ITreeItem::Ptr>, bool del)>;

        MultiSelection();
        bool hasItem(ITreeItem::Ptr node) const;
        void updateItem(ITreeItem::Ptr node, bool v);
        void clear();

        void applyRequests(ImGuiMultiSelectIO* multiSelectIo, std::vector<ITreeItem::Ptr>& items, int items_count);
        void processDeletionPreLoop(ImGuiMultiSelectIO*, int itemToFocus);
        void processDeletionPostLoop(ImGuiMultiSelectIO*, std::vector<ITreeItem::Ptr>& items, int itemToFocus);
        bool hasObjectsToDelete() const;

        void setMultiItemCallback(MultiItemCallback&& multiItemCallback) {
            m_multiItemCallback = multiItemCallback;
        }
        void setMultiItemRangeCallback(MultiItemRangeCallback && callback) {
            m_multiRangeItemCallback = callback;
        }
    private:
        void addItem(ImGuiID key);
        void removeItem(ImGuiID key);

    private:
        ImGuiStorage m_storage;
        bool m_queryDeletion{false};
        int m_size{0};

        ImGuiID (*AdapterIndexToStorageId)(MultiSelection* self, ITreeItem::Ptr node);

        MultiItemCallback m_multiItemCallback;
        MultiItemRangeCallback m_multiRangeItemCallback;
    };

    MultiSelection::MultiSelection() {
        AdapterIndexToStorageId = [](MultiSelection*, ITreeItem::Ptr node) { return (ImGuiID)node -> getID(); };
    }

    void MultiSelection::addItem(ImGuiID key) {
        int* p_int = m_storage.GetIntRef(key, 0);
        if (*p_int != 0)
            return;
        *p_int = 1;
        m_size++;
    }

    void MultiSelection::updateItem(ITreeItem::Ptr node, bool v) {
        ImGuiID key = AdapterIndexToStorageId(this, node);

        if (v)
            addItem(key);
        else
            removeItem(key);
    }

    void MultiSelection::removeItem(ImGuiID key) {
        int* p_int = m_storage.GetIntRef(key, 0);
        if (*p_int == 0) return;
        *p_int = 0;
        m_size--;
    }

    bool MultiSelection::hasItem(ITreeItem::Ptr node) const {
        ImGuiID key = AdapterIndexToStorageId(const_cast<MultiSelection *>(this), node);
        return m_storage.GetInt(key, 0) != 0;
    }

    void MultiSelection::clear() {
        m_queryDeletion = false;
        m_storage.Data.resize(0);
        m_size = 0;
    }

    void MultiSelection::applyRequests(ImGuiMultiSelectIO* multiSelectIo, std::vector<ITreeItem::Ptr>& items, int items_count) {

        for(auto& request: multiSelectIo -> Requests) {
            if (request.Type == ImGuiSelectionRequestType_Clear)
                clear();
            if (request.Type == ImGuiSelectionRequestType_SelectAll)
            {
                m_storage.Data.resize(0);
                m_storage.Data.reserve(items_count);
                for(auto& item: items)
                    addItem(AdapterIndexToStorageId(this, item));

                m_multiItemCallback(items);
            }
            if (request.Type == ImGuiSelectionRequestType_SetRange) {
                std::vector<ITreeItem::Ptr> rangeItems{};

                auto firstFound = std::find_if(items.begin(), items.end(),
                                               [&request](const ITreeItem::Ptr& ptr) {
                    return ptr -> getID() == static_cast<UUID>(request.RangeFirstItem);
                });

                auto lastFound = std::find_if(items.begin(), items.end(),
                                              [&request](const ITreeItem::Ptr& ptr) {
                    return ptr -> getID() == static_cast<UUID>(request.RangeLastItem);
                });

                if(lastFound != items.end()) {
                    for(; firstFound <= lastFound; ++firstFound) {
                        updateItem(*firstFound, request.RangeSelected);
                        rangeItems.push_back(*firstFound);
                    }
                }
                else
                    updateItem(*firstFound, request.RangeSelected);


                if(request.RangeFirstItem != request.RangeLastItem)
                    m_multiRangeItemCallback(rangeItems, request.RangeSelected);
            }
        }

    }

    void MultiSelection::processDeletionPreLoop(ImGuiMultiSelectIO* ms_io, int itemToFocus) {

    }

    void MultiSelection::processDeletionPostLoop(ImGuiMultiSelectIO* ms_io, std::vector<ITreeItem::Ptr>& items, int itemToFocus) {

    }

    bool MultiSelection::hasObjectsToDelete() const {
        return !m_queryDeletion && !m_storage.Data.empty();
    }

    MultiSelection m_multiSelection;

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
        for(auto item: m_setItems) {
            auto iter = std::get<0>(item);
            if(iter == m_items.begin())
                m_items.insert(iter, std::get<1>(item));
            else
                m_items.insert(iter - 1, std::get<1>(item));
        }
        m_setItems.clear();

        for(auto item: m_insertItems) {
            auto source = std::get<1>(item);
            m_items.insert(std::get<0>(item), source);

            if(std::get<2>(item) == ReorderDeleteType::Last) {
                auto found = util::find_last_if(m_items.begin(), m_items.end(), [&source](ITreeItem::Ptr item) {
                    return item -> m_id == source->m_id;
                });
                m_items.erase(found);
            }
            else if(std::get<2>(item) == ReorderDeleteType::First) {
                auto found = util::find_first_if(m_items.begin(), m_items.end(), [&source](ITreeItem::Ptr item) {
                    return item -> m_id == source -> m_id;
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



        m_multiSelection.setMultiItemCallback(std::move(m_multiItemCallback));
        m_multiSelection.setMultiItemRangeCallback(std::move(m_multiItemRangeCallback));

        static ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_None | ImGuiMultiSelectFlags_ClearOnEscape;

        if(ImGui::TreeNodeEx(m_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

            ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags);
            m_multiSelection.applyRequests(ms_io, m_items, m_items.size());

            const bool wantDelete = m_multiSelection.hasObjectsToDelete()
                    && ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete);
            int item_curr_idx_to_focus = -1;

            if(wantDelete) {
                m_multiSelection.processDeletionPreLoop(ms_io, -1);
            }

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
#ifdef _WIN32
                                    RB_EDITOR_INFO("Treehierachy: got item with ID = {0}", id);
#else
                                    // TODO(a.raag) fmt don't support UUID Correctly
#endif
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
#ifdef _WIN32
                                        RB_EDITOR_INFO("TreeHierachy: Want make as child item with ID = {0}", id);
#else
                                        // TODO(a.raag) fmt don't support UUID Correctly
#endif
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

                    //node_flags |= ((m_selectedID == item -> getID()) ? ImGuiTreeNodeFlags_Selected : 0);
                    node_flags |= ((m_multiSelection.hasItem(item)) ? ImGuiTreeNodeFlags_Selected : 0);


                    // TODO(a.raag) node item Ptr
                    ImGui::SetNextItemSelectionUserData(item -> getID());

                    auto uuid = item -> m_id;

                    constexpr auto drawTexture = [](robot2D::vec2f size, robot2D::Texture* texture,
                                robot2D::Color tintColor) {
                        auto imID = ImGui::convertTextureHandle(texture -> getID());
                        ImGui::ImageButton(imID, size, {0,1}, {1, 0}, -1, {0, 0, 0, 0}, tintColor);
                    };

                    if(item -> m_iconTexture) {
                        drawTexture({20, 20}, item -> m_iconTexture, item -> m_tintColor);
                        ImGui::SameLine();
                    }

                    bool node_open = ImGui::TreeNodeEx(static_cast<void*>(&uuid), node_flags,
                                                       item -> m_name -> c_str());

                    if(ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                        m_selectedID = item -> m_id;
                        m_childSelectedID = NO_INDEX;
                        m_selectCallback(item);
                    }
                    else if(ImGui::IsItemClicked(ImGuiMouseButton_Right))
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
                }
            }

            ms_io = ImGui::EndMultiSelect();
            m_multiSelection.applyRequests(ms_io, m_items, m_items.size());

            if(wantDelete) {
                m_multiSelection.processDeletionPostLoop(ms_io, m_items, item_curr_idx_to_focus);
            }

            ImGui::TreePop();
        }

    }

    ITreeItem::Ptr TreeHierarchy::findByID(UUID ID) {
        ITreeItem::Ptr parent = nullptr;
        auto found = std::find_if(m_items.begin(), m_items.end(),
                                  [ID, &parent](ITreeItem::Ptr item) {
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

        /// TODO(a.raag): make key from Item
        m_multiSelection.updateItem(item, true);
    }

    void TreeHierarchy::clearSelection() {
        m_multiSelection.clear();
    }

}