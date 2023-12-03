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
#include <functional>
#include <imgui/imgui.h>
#include "ITreeItem.hpp"

namespace editor {

    class MultiSelectionStorageAdapter {
    public:
        MultiSelectionStorageAdapter() {
            m_AdapterIndexToStorageId = [](MultiSelectionStorageAdapter*, ITreeItem::Ptr node) {
                return static_cast<ImGuiID>(node -> getID());
            };
        }
        ~MultiSelectionStorageAdapter() = default;

        void reserve(int size) {
            if(m_mode == Mode::ImGui) {
                m_ImGuistorage.Data.resize(0);
                m_ImGuistorage.Data.reserve(size);
            }
            else {

            }

        }

        bool hasItem(ITreeItem::Ptr node) const {
            if(m_mode == Mode::ImGui) {
                ImGuiID key = m_AdapterIndexToStorageId(const_cast<MultiSelectionStorageAdapter *>(this), node);
                return m_ImGuistorage.GetInt(key, 0) != 0;
            }
            else {
                return false;
            }

        }

        bool addItem(ITreeItem::Ptr node) {
            if(m_mode == Mode::ImGui) {
                ImGuiID key = m_AdapterIndexToStorageId(const_cast<MultiSelectionStorageAdapter *>(this), node);
                int* p_int = m_ImGuistorage.GetIntRef(key, 0);
                if (*p_int != 0)
                    return false;
                *p_int = 1;
                return true;
            }
            else {
                return true;
            }
        }

        bool removeItem(ITreeItem::Ptr node) {
            if(m_mode == Mode::ImGui) {
                ImGuiID key = m_AdapterIndexToStorageId(const_cast<MultiSelectionStorageAdapter *>(this), node);
                int* p_int = m_ImGuistorage.GetIntRef(key, 0);
                if (*p_int == 0)
                    return false;
                *p_int = 0;
                return true;
            }
            else {
                return true;
            }

        }

        void clear() {
            if(m_mode == Mode::ImGui) {
                m_ImGuistorage.Data.resize(0);
            }
            else {
                m_storage.clear();
            }
        }

        bool empty() const {
            if(m_mode == Mode::ImGui) {
                return m_ImGuistorage.Data.empty();
            }
            else {
                return false;
            }
        }
    private:
        enum class Mode {
            ImGui,
            Custom
        } m_mode = Mode::ImGui;


        ImGuiID (*m_AdapterIndexToStorageId)(MultiSelectionStorageAdapter* self, ITreeItem::Ptr node);

        ImGuiStorage m_ImGuistorage;
        std::vector<UUID> m_storage;
    };

    class MultiSelection {
    public:
        using MultiItemCallback = std::function<void(std::vector<ITreeItem::Ptr>)>;
        using MultiItemRangeCallback = std::function<void(std::vector<ITreeItem::Ptr>, bool del)>;

        MultiSelection();
        ~MultiSelection() = default;

        [[nodiscard]] bool hasItem(ITreeItem::Ptr node) const;
        void updateItem(ITreeItem::Ptr node, bool needAdd);
        void clear();

        void applyRequests(ImGuiMultiSelectIO* multiSelectIo, std::list<ITreeItem::Ptr>& items);
        ITreeItem::Ptr processDeletionPreLoop(ImGuiMultiSelectIO*, std::list<ITreeItem::Ptr>& items);
        void processDeletionPostLoop(ImGuiMultiSelectIO*, std::list<ITreeItem::Ptr>& items,
                                        ITreeItem::Ptr item_next_idx_to_select);

        void setMultiItemCallback(MultiItemCallback&& multiItemCallback) {
            m_multiItemCallback = multiItemCallback;
        }
        void setMultiItemRangeCallback(MultiItemRangeCallback && callback) {
            m_multiRangeItemCallback = callback;
        }

        void setQueueDeletion() { m_queueDeletion = true; }

        bool hasQueryDeletion() const { return m_queueDeletion; }
        bool empty() const { return m_size == 0; }
    private:
        void addItem(ITreeItem::Ptr node);
        void removeItem(ITreeItem::Ptr node);

    private:
        MultiSelectionStorageAdapter m_storage;

        bool m_queueDeletion{false};
        int m_size{0};

        MultiItemCallback m_multiItemCallback;
        MultiItemRangeCallback m_multiRangeItemCallback;
    };
}