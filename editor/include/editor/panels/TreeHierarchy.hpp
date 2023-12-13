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
#include <vector>
#include <string>
#include <memory>
#include <tuple>
#include <algorithm>
#include <functional>
#include <limits>
#include <list>

#include <imgui/imgui.h>
#include <robot2D/Core/Keyboard.hpp>
#include <robot2D/Graphics/Texture.hpp>
#include <editor/Uuid.hpp>

#include "ITreeItem.hpp"
#include "MultiSelection.hpp"

namespace editor {

    template<typename T>
    class UserDataStorage {
    public:


    private:
        std::aligned_storage<sizeof(T), alignof(T)> m_storage;
    };

    template<typename UserDataT, typename = std::enable_if_t<std::is_standard_layout_v<UserDataT>>>
    class TreeItem: public ITreeItem {
    private:
        using TreeItemUserData = std::shared_ptr<UserDataT>;
    public:
        using Ptr = std::shared_ptr<TreeItem<UserDataT>>;
        TreeItem(): ITreeItem() { }
        explicit TreeItem(int id): ITreeItem(id) { }

        template<typename... Args>
        TreeItem::Ptr addChild(Args&& ... args) {
            auto child = std::make_shared<TreeItem<UserDataT>>(std::forward<Args>(args)...);
            child -> m_id = m_id;
            child -> m_child_id = UUID();
            child -> m_parent = this;
            m_childrens.template emplace_back(child);
            return child;
        }

        void setUserData(UserDataT value) {
            m_userData = std::make_shared<UserDataT>(std::move(value));
            if(!m_userData) {
                /// Error
            }
        }

        friend bool operator ==(const TreeItem<UserDataT>& left, const TreeItem<UserDataT>& right);
        friend bool operator !=(const TreeItem<UserDataT>& left, const TreeItem<UserDataT>& right);
    protected:
        void* getUserDataInternal() const override {
            assert(m_userData != nullptr && "before get data store it, using SetUserData Method");
            return static_cast<void*>(m_userData.get());
        }
    private:
        /// TODO(a.raag): add UserDataStorage
        TreeItemUserData m_userData{nullptr};
    };


    template<typename T>
    bool operator ==(const TreeItem<T>& left, const TreeItem<T>& right) {
        return ((left.m_id == right.m_id) && (left.m_name == right.m_name));
    }

    template<typename T>
    bool operator !=(const TreeItem<T>& left, const TreeItem<T>& right) {
        return !(left == right);
    }

    namespace util {
        template <class BidirectionalIterator, class T> inline
        BidirectionalIterator find_first_if(const BidirectionalIterator first, const BidirectionalIterator last, T&& predicate)
        {
            for (BidirectionalIterator it = first; it != last;)
                //reverse iteration: 1. check 2. decrement 3. evaluate
            {
                if (predicate(*it))
                    return it;
                ++it; //
            }
            return first;
        }

        template <class BidirectionalIterator, class T> inline
        BidirectionalIterator find_last_if(const BidirectionalIterator first, const BidirectionalIterator last, T&& predicate)
        {
            for (BidirectionalIterator it = last; it != first;)
                //reverse iteration: 1. check 2. decrement 3. evaluate
            {
                --it; //

                if (predicate(*it))
                    return it;
            }
            return last;
        }
    }

    /// TODO(a.raag): Add as Child, and remove be child Logic
    class TreeHierarchy {
    public:
        explicit TreeHierarchy(std::string name): m_name(std::move(name)) {
            m_tree_base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                | ImGuiTreeNodeFlags_SpanAvailWidth;
        }
        ~TreeHierarchy() = default;

        using ItemCallback = std::function<void(ITreeItem::Ptr selected)>;
        using ReorderItemCallback = std::function<void(ITreeItem::Ptr source, ITreeItem::Ptr target)>;
        using MultiItemCallback = std::function<void(std::list<ITreeItem::Ptr>)>;
        using MultiItemRangeCallback = std::function<void(std::vector<ITreeItem::Ptr>, bool del)>;
        using InsertItemCallback = std::function<void(ITreeItem::Ptr inserted)>;

        template<typename T, typename ... Args>
        typename TreeItem<T>::Ptr addItem(bool needPending = false, Args&& ... args) {
            static_assert(std::is_standard_layout_v<T> && "UserData Type must be pod");
            auto treeItem = std::make_shared<TreeItem<T>>(std::forward<Args>(args)...);
            if(!treeItem) {
                /// error
            }
            treeItem -> m_id = UUID();
            if(!needPending) {
                m_items.template emplace_back(treeItem);
                m_multiSelection.updateItem(treeItem, true);
            }
            else
                m_additemsBuffer.emplace_back(treeItem);
            return treeItem;
        }

        bool deleteItem(ITreeItem::Ptr treeItem);


        bool addOnSelectCallback(ItemCallback&& callback);

        bool addOnCallback(ItemCallback&& callback);

        bool addOnReorderCallback(ReorderItemCallback&& callback);

        bool addOnMakeChildCallback(ReorderItemCallback&& callback);

        bool addOnStopBeChildCallback(ReorderItemCallback&& callback);

        bool addMultiSelectCallback(MultiItemCallback&& callback);

        bool addMultiSelectRangeCallback(MultiItemRangeCallback&& callback);

        bool addInsertItemCallback(InsertItemCallback&& callback);

        template<typename T>
        TreeItem<T>* getItem(UUID id) {
            auto found = std::find_if(m_items.begin(), m_items.end(), [&id](ITreeItem::Ptr ptr) {
                return ptr -> m_id == id;
            });
            if(found == m_items.end())
                return nullptr;
            return dynamic_cast<TreeItem<T>*>(*found->get());
        }

        void setBefore(ITreeItem::Ptr source, ITreeItem::Ptr target);

        void insertItem(ITreeItem::Ptr source, ITreeItem::Ptr anchor, bool first = false, bool isChained = false);


        template<typename T>
        T* getDataByItem(UUID ID) {
            if(auto item = findByID(ID)) {
                return item -> template getUserData<T>();
            }
            return nullptr;
        }

        void setSelected(ITreeItem::Ptr item);
        void clearSelection();

        std::list<ITreeItem::Ptr>& getItems() { return m_items; }
        const std::list<ITreeItem::Ptr>& getItems() const { return m_items; }

        void clear();
        void render();
    private:
        ITreeItem::Ptr findByID(UUID ID);
        void renderTree();
    private:
        std::string m_name;
        std::list<ITreeItem::Ptr> m_items{};
        std::list<ITreeItem::Ptr> m_deletePendingItems{};
        std::list<ITreeItem::Ptr> m_additemsBuffer{};

        enum class ReorderDeleteType {
            First, Last
        };

        using Iterator = std::list<ITreeItem::Ptr>::iterator;
        using InsertItem = std::tuple<Iterator, ITreeItem::Ptr, ReorderDeleteType>;
        using SetItem = std::tuple<Iterator, ITreeItem::Ptr, bool, ITreeItem::Ptr>;

        int m_tree_base_flags = 0;
        UUID m_selectedID = NO_INDEX;
        UUID m_childSelectedID = NO_INDEX;

        ItemCallback m_selectCallback;
        ItemCallback m_callback;
        ReorderItemCallback m_reorderCallback;
        ReorderItemCallback m_makeAsChildCallback;
        ReorderItemCallback m_removeAsChildCallback;
        MultiItemCallback m_multiItemCallback;
        MultiItemRangeCallback m_multiItemRangeCallback;
        InsertItemCallback  m_insertItemCallback;

        std::vector<InsertItem> m_insertItems;
        std::vector<SetItem> m_setItems;


        robot2D::Key m_shortCutKey{robot2D::Key::Q};
        std::string m_playloadIdentifier = "TreeNodeItem";
        MultiSelection m_multiSelection;
    };


}