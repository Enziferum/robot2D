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

#pragma once
#include <list>
#include <set>
#include <functional>

#include "ITreeItem.hpp"

namespace editor {
    class MultiSelection {
    public:
        using MultiItemCallback = std::function<void(std::set<ITreeItem::Ptr>&, bool)>;

        MultiSelection() = default;
        MultiSelection(const MultiSelection& other) = delete;
        MultiSelection& operator=(const MultiSelection& other) = delete;
        MultiSelection(MultiSelection&& other) = delete;
        MultiSelection& operator=(MultiSelection&& other) = delete;
        ~MultiSelection() = default;


        bool hasItem(ITreeItem::Ptr item) const {
            return std::find_if(m_selectedItems.begin(), m_selectedItems.end(), [&item](ITreeItem::Ptr obj) {
                return *item == *obj;
            }) != m_selectedItems.end();
        }

        void setMultiItemCallback(MultiItemCallback&& callback) { m_callback = std::move(callback); }

        void preUpdate(std::list<ITreeItem::Ptr>& items);
        void update();
        void postUpdate();
        /// \brief Using only due to Hierarchy process, to outside update use AddItem() / removeItem()
        void markSelected(ITreeItem::Ptr item);

        void addItem(ITreeItem::Ptr item);

        void removeItem(ITreeItem::Ptr item);
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
        MultiItemCallback m_callback { nullptr };
    };
}