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

#include <editor/panels/MultiSelection.hpp>

namespace editor {


    MultiSelection::MultiSelection() {}

    void MultiSelection::addItem(ITreeItem::Ptr node) {
        if(m_storage.addItem(node))
            m_size++;
    }

    void MultiSelection::updateItem(ITreeItem::Ptr node, bool needAdd) {
        if (needAdd)
            addItem(node);
        else
            removeItem(node);
    }

    void MultiSelection::removeItem(ITreeItem::Ptr node) {
        if(m_storage.removeItem(node))
            m_size--;
    }

    bool MultiSelection::hasItem(ITreeItem::Ptr node) const {
        if(!node)
            return false;

        return m_storage.hasItem(node);
    }

    void MultiSelection::clear() {
        m_queueDeletion = false;
        m_storage.clear();
        m_size = 0;
    }

    void MultiSelection::applyRequests(ImGuiMultiSelectIO* multiSelectIo, std::list<ITreeItem::Ptr>& items) {

        for(auto& request: multiSelectIo -> Requests) {
            if (request.Type == ImGuiSelectionRequestType_Clear)
                clear();
            if (request.Type == ImGuiSelectionRequestType_SelectAll)
            {
                m_storage.reserve(static_cast<int>(items.size()));
                for(auto& item: items)
                    addItem(item);

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
                    for(; firstFound != lastFound; ++firstFound) {
                        updateItem(*firstFound, request.RangeSelected);
                        rangeItems.push_back(*firstFound);
                    }
                    updateItem(*lastFound, request.RangeSelected);
                    rangeItems.push_back(*lastFound);
                }
                else if(firstFound != items.end())
                    updateItem(*firstFound, request.RangeSelected);


                if(request.RangeFirstItem != request.RangeLastItem)
                    m_multiRangeItemCallback(rangeItems, request.RangeSelected);
            }
        }

    }

    ITreeItem::Ptr MultiSelection::processDeletionPreLoop(ImGuiMultiSelectIO* ms_io, std::list<ITreeItem::Ptr>& items) {
        return nullptr;
        m_queueDeletion = false;

        // If focused item is not selected...
//
//        UUID focusedUUID = static_cast<UUID>(ms_io -> NavIdItem); // Index of currently focused item
//        auto focusedItem = std::find_if(items.begin(), items.end(),
//                                        [&focusedUUID](ITreeItem::Ptr item) {
//                                            return focusedUUID == item -> getID();
//                                        });
//        if (ms_io -> NavIdSelected == false)  // This is merely a shortcut, == Contains(adapter->IndexToStorage(items, focused_idx))
//        {
//            ms_io -> RangeSrcReset = true;    // Request to recover RangeSrc from NavId next frame. Would be ok to reset even when NavIdSelected==true, but it would take an extra frame to recover RangeSrc when deleting a selected item.
//            if(focusedItem != items.end())
//                return *focusedItem;  // Request to focus same item after deletion.
//            return nullptr;
//        }
//
//        if(focusedItem == items.end() || (focusedItem + 1) == items.end())
//            return nullptr;
//
//        // If focused item is selected: land on first unselected item after focused item.
//        for(auto idx = focusedItem + 1; idx != items.end(); ++idx)
//            if(!hasItem(*idx))
//                return (*idx);
//
//        // If focused item is selected: otherwise return last unselected item before focused item.
//        for(auto idx = focusedItem - 1; idx >= items.begin(); --idx)
//            if(!hasItem(*idx))
//                return (*idx);

        return nullptr;
    }

    void MultiSelection::processDeletionPostLoop(ImGuiMultiSelectIO* ms_io, std::list<ITreeItem::Ptr>& items,
                                                 ITreeItem::Ptr item_curr_idx_to_select) {
        return;
        // Rewrite item list (delete items) + convert old selection index (before deletion) to new selection index (after selection).
        // If NavId was not part of selection, we will stay on same item.
//        std::vector<ITreeItem::Ptr> newItems;
//        newItems.reserve(items.size() - m_size);
//        ITreeItem::Ptr item_next_idx_to_select = nullptr;
//        for(auto& item: items) {
//            if (!hasItem(item))
//                newItems.push_back(item);
//            if (item_curr_idx_to_select == item)
//                item_next_idx_to_select = *(items.end() - 1);
//        }
//
//        items.swap(newItems);
//
//        // Update selection
//        clear();
//        if (item_next_idx_to_select != nullptr && ms_io->NavIdSelected)
//            addItem(item_next_idx_to_select);
    }


}