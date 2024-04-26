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

    namespace util {
        template <class BidirectionalIterator, class T> inline
            BidirectionalIterator find_first_if(const BidirectionalIterator first,
                const BidirectionalIterator last, T&& predicate)
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
            BidirectionalIterator find_last_if(const BidirectionalIterator first,
                const BidirectionalIterator last, T&& predicate)
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
        TreeItem(const TreeItem& other) = delete;
        TreeItem& operator=(const TreeItem& other) = delete;
        TreeItem(TreeItem&& other) = delete;
        TreeItem& operator=(TreeItem&& other) = delete;
        ~TreeItem() override = default;

        template<typename... Args>
        TreeItem::Ptr addChild(Args&& ... args) {
            auto child = std::make_shared<TreeItem<UserDataT>>(std::forward<Args>(args)...);
            child -> m_id = UUID();
            child -> m_parent = this;
            child -> m_path.append(m_childrens.size());
            child -> m_internalID = m_childrens.size();
            m_childrens.emplace_back(child);
            return child;
        }

        void setUserData(UserDataT value) {
            m_userData = std::make_shared<UserDataT>(std::move(value));
            if(!m_userData) {
                /// Error
            }
        }

        template<typename T>
        friend bool operator ==(const TreeItem<T>& left, const TreeItem<T>& right);
        template<typename T>
        friend bool operator !=(const TreeItem<T>& left, const TreeItem<T>& right);
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


    /// TODO(a.raag): Add as Child, and remove be child Logic
    class TreeHierarchy {
    private:
        using ItemCallback = std::function<void(ITreeItem::Ptr selected)>;
        using ReorderItemCallback = std::function<void(ITreeItem::Ptr source, ITreeItem::Ptr target)>;
        using MultiItemCallback = std::function<void(std::set<ITreeItem::Ptr>&, bool)>;
        using Iterator = std::list<ITreeItem::Ptr>::iterator;
        enum class ReorderDeleteType {
            First,
            Last
        };
        using InsertItem = std::tuple<Iterator, ITreeItem::Ptr, ReorderDeleteType>;
    public:
        struct RestoreInfo {
            bool isChained;
            bool isFirst;
            ITreeItem::Ptr target;
            ITreeItem::Ptr anchor;

        private:
            friend class TreeHierarchy;
            Iterator m_sourceIterator;
        };

        explicit TreeHierarchy(std::string name);
        TreeHierarchy(const TreeHierarchy& other) = delete;
        TreeHierarchy& operator=(const TreeHierarchy& other) = delete;
        TreeHierarchy(TreeHierarchy&& other) = delete;
        TreeHierarchy& operator=(TreeHierarchy&& other) = delete;
        ~TreeHierarchy() = default;


        template<typename T, typename ... Args>
        typename TreeItem<T>::Ptr addItem(bool needPending = false, Args&& ... args) {
            // TODO(a.raag): robot2D::ecs::Entity is not is_standard_layout_v on MSVC
            //static_assert(std::is_standard_layout_v<T> && "UserData Type must be pod");
            
            auto treeItem = std::make_shared<TreeItem<T>>(std::forward<Args>(args)...);
            if(!treeItem) {
                //RB_EDITOR_ERROR("TreeHierarchy: can't allocate TreeItem");
                return nullptr;
            }
            treeItem -> m_id = UUID();
            treeItem -> m_path.append(m_items.size());
            treeItem -> m_internalID = m_items.size();

            if(!needPending)
                m_items.emplace_back(treeItem);
            else
                m_additemsBuffer.emplace_back(treeItem);
            return treeItem;
        }

        ITreeItem::Ptr getItem(UUID uuid) {
            return findByID(uuid);
        }

        template<typename T>
        T* getDataByItem(UUID ID) {
            if (auto item = findByID(ID)) {
                return item -> template getUserData<T>();
            }
            return nullptr;
        }

        //////////////////////////////////////// Callbacks ////////////////////////////////////////
        bool addOnSelectCallback(ItemCallback&& callback);
        bool addOnCallback(ItemCallback&& callback);
        bool addOnReorderCallback(ReorderItemCallback&& callback);
        bool addOnMakeChildCallback(ReorderItemCallback&& callback);
        bool addOnStopBeChildCallback(ReorderItemCallback&& callback);
        bool addMultiSelectCallback(MultiItemCallback&& callback);
        //////////////////////////////////////// Callbacks ////////////////////////////////////////

        void setBefore(ITreeItem::Ptr source, ITreeItem::Ptr target);
        void restoreItem(RestoreInfo&& restoreInfo);
        bool deleteItem(ITreeItem::Ptr treeItem);

        void setSelected(ITreeItem::Ptr item);
        void clearSelection();

        std::list<ITreeItem::Ptr>& getItems() { return m_items; }
        const std::list<ITreeItem::Ptr>& getItems() const { return m_items; }
        const std::size_t getItemsValue() const;

        void applyChildModification(ITreeItem::Ptr source, ITreeItem::Ptr target);


        void clear();
        void render();
    private:
        [[nodiscard]] ITreeItem::Ptr findByID(UUID ID) const;
        [[nodiscard]] ITreeItem::Ptr findByID(ITreeItem::Ptr Parent, UUID ID) const;

        void update();
        void renderTree();
        void renderTreeChildren(ITreeItem::Ptr parent);
    private:
        int m_tree_base_flags = 0;
        std::string m_name;
        std::list<ITreeItem::Ptr> m_items{};
        std::list<ITreeItem::Ptr> m_deletePendingItems{};
        std::list<ITreeItem::Ptr> m_additemsBuffer{};


        ItemCallback m_selectCallback;
        ItemCallback m_callback;
        ReorderItemCallback m_reorderCallback;
        ReorderItemCallback m_makeAsChildCallback;
        ReorderItemCallback m_removeAsChildCallback;
        MultiItemCallback m_multiItemCallback;

        std::vector<InsertItem> m_insertItems;
        std::vector<RestoreInfo> m_restoreItems;


        robot2D::Key m_shortCutKey{robot2D::Key::Q};
        std::string m_playloadIdentifier = "TreeNodeItem";
        MultiSelection m_multiSelection;

        ITreeItem::Ptr m_source{nullptr};
        ITreeItem::Ptr m_target{nullptr};
    };


}