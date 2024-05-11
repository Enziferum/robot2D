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

#include <algorithm>
#include <imgui/imgui.h>
#include <editor/panels/MultiSelection.hpp>

namespace editor {

    void MultiSelection::preUpdate(std::list<ITreeItem::Ptr>& items) {

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
                bool selectedAll = false;
                if(m_callback)
                    m_callback(m_selectedItems, selectedAll);
                break;
            }
            case State::MultiAllSelect: {
                for(auto& item: items)
                    m_selectedItems.insert(item);

                bool selectedAll = true;
                if(m_callback)
                    m_callback(m_selectedItems, selectedAll);
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

                    /// TODO(a.raag): swap first and last if last < first

                    auto firstGraphPath = (*firstFound)->getNodePath();
                    auto lastGraphPath = (*lastFound)->getNodePath();

                    if (!firstGraphPath.isSameLevel(lastGraphPath))
                        break;

                    if((*firstFound) > (*lastFound))
                        std::swap(firstFound, lastFound);

                    if(firstFound != items.end() && lastFound != items.end()) {
                        for(; firstFound != lastFound; ++firstFound)
                            m_selectedItems.insert(*firstFound);
                        m_selectedItems.insert(*lastFound);


                        bool selectedAll = false;
                        if(m_callback)
                            m_callback(m_selectedItems, selectedAll);
                    }

                }

                break;
            }
        }

        m_preSelectedItems.clear();
    }


    void MultiSelection::update() {
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

    void MultiSelection::postUpdate() {
        bool needClear = false;
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()
           && !m_selectedItems.empty() && !(m_currentState == State::MultiSelect
                                            || m_currentState == State::RangeSelect))
            needClear = true;

        if(ImGui::IsKeyPressed(ImGuiKey_Escape) || needClear)
            m_currentState = State::Clear;
    }

    void MultiSelection::markSelected(ITreeItem::Ptr item) {

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

    void MultiSelection::clearAll() {
        m_preSelectedItems.clear();
        m_selectedItems.clear();
    }

    void MultiSelection::addItem(ITreeItem::Ptr item) {
        m_selectedItems.insert(item);
    }

    void MultiSelection::removeItem(ITreeItem::Ptr item) {
        m_selectedItems.erase(item);
    }
}