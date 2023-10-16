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

#include <imgui/imgui.h>
#include <robot2D/Core/Keyboard.hpp>
#include <editor/Uuid.hpp>

namespace editor {
    constexpr std::uint64_t NO_INDEX = std::numeric_limits<uint64_t>::max();

    class ITreeItem {
    public:
        using Ptr = std::shared_ptr<ITreeItem>;

        ITreeItem(): m_id() {}
        ITreeItem(UUID id): m_id{id} {}
        virtual ~ITreeItem() = 0;

        bool hasChildrens() const { return !m_childrens.empty(); }

        UUID getID() const { return m_id; }

        void setName(std::string& name) {
            m_name = &name;
        }

        /// \brief don't call if user data was set void or you want to call on void type
        template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, void>>>
        T* getUserData() const {
            return static_cast<T*>(getUserDataInternal());
        }

        bool isChild() const { return m_child_id != NO_INDEX; }

        bool deleteChild(ITreeItem::Ptr item) {
            m_deletePendingItems.emplace_back(item);
            return true;
        }

        void addChild(ITreeItem::Ptr child) {
            child -> m_parent = this;
            child -> m_id = NO_INDEX;
            child -> m_child_id = UUID();
            m_childrens.emplace_back(child);
        }

        void removeSelf() {
            if(!isChild() || !m_parent)
                return;
            m_parent -> removeChild(this);
        }

        std::vector<ITreeItem::Ptr>& getChildrens() { return m_childrens; }
    protected:
        void removeChild(ITreeItem* child) {
            auto found = std::find_if(m_childrens.begin(), m_childrens.end(), [&child](ITreeItem::Ptr item) {
                return child -> m_child_id == item -> m_child_id;
            });
            m_childrens.erase(found, m_childrens.end());
        }

        virtual void* getUserDataInternal() const = 0;
        UUID m_id;
        UUID m_child_id{NO_INDEX};

        std::string* m_name{nullptr};
        ITreeItem* m_parent{nullptr};
        std::vector<ITreeItem::Ptr> m_childrens;
        std::vector<ITreeItem::Ptr> m_deletePendingItems{};

        friend class TreeHierarchy;
    };



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
        using MultiItemCallback = std::function<void(std::vector<ITreeItem::Ptr>)>;


        template<typename T, typename ... Args>
        typename TreeItem<T>::Ptr addItem(bool needPending = false, Args&& ... args) {
            static_assert(std::is_standard_layout_v<T> && "UserData Type must be pod");
            auto treeItem = std::make_shared<TreeItem<T>>(std::forward<Args>(args)...);
            if(!treeItem) {
                /// error
            }
            treeItem -> m_id = UUID();
            if(!needPending)
                m_items.template emplace_back(treeItem);
            else
                m_additemsBuffer.emplace_back(treeItem);
            return treeItem;
        }

        bool deleteItem(ITreeItem::Ptr treeItem);

        bool addOnSelectCallback(ItemCallback&& callback) {
            if(callback)
                m_selectCallback = std::move(callback);
            else
                return false;
            return true;
        }

        bool addOnCallback(ItemCallback&& callback) {
            if(callback)
                m_callback = std::move(callback);
            else
                return false;
            return true;
        }

        bool addOnReorderCallback(ReorderItemCallback&& callback) {
            if(callback)
                m_reorderCallback = std::move(callback);
            else
                return false;
            return true;
        }

        bool addOnMakeChildCallback(ReorderItemCallback&& callback) {
            if(callback)
                m_makeAsChildCallback = std::move(callback);
            else
                return false;
            return true;
        }

        bool addOnStopBeChildCallback(ReorderItemCallback&& callback) {
            if(callback)
                m_removeAsChildCallback = std::move(callback);
            else
                return false;
            return true;
        }

        bool addMultiSelectCallback(MultiItemCallback&& callback) {
            if(callback)
                m_multiItemCallback = std::move(callback);
            else
                return false;
            return true;
        }

        template<typename T>
        TreeItem<T>* getItem(UUID id) {
            auto found = std::find_if(m_items.begin(), m_items.end(), [&id](ITreeItem::Ptr ptr) {
                return ptr -> m_id == id;
            });
            if(found == m_items.end())
                return nullptr;
            return dynamic_cast<TreeItem<T>*>(*found->get());
        }

        void setBefore(ITreeItem::Ptr source, ITreeItem::Ptr target) {
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

        template<typename T>
        T* getDataByItem(UUID ID) {
            if(auto item = findByID(ID)) {
                return item -> template getUserData<T>();
            }
            return nullptr;
        }

        void setSelected(ITreeItem::Ptr item);

        std::vector<ITreeItem::Ptr>& getItems() { return m_items; }
        const std::vector<ITreeItem::Ptr>& getItems() const { return m_items; }

        void clear();
        void render();
    private:
        ITreeItem::Ptr findByID(UUID ID);

    private:
        std::string m_name;
        std::vector<ITreeItem::Ptr> m_items{};
        std::vector<ITreeItem::Ptr> m_deletePendingItems{};
        std::vector<ITreeItem::Ptr> m_additemsBuffer{};

        enum class ReorderDeleteType {
            First, Last
        };

        using Iterator = std::vector<ITreeItem::Ptr>::iterator;
        using InsertItem = std::tuple<Iterator, ITreeItem::Ptr, ReorderDeleteType>;

        int m_tree_base_flags = 0;
        UUID m_selectedID = NO_INDEX;
        UUID m_childSelectedID = NO_INDEX;

        ItemCallback m_selectCallback;
        ItemCallback m_callback;
        ReorderItemCallback m_reorderCallback;
        ReorderItemCallback m_makeAsChildCallback;
        ReorderItemCallback m_removeAsChildCallback;
        MultiItemCallback m_multiItemCallback;


        std::vector<InsertItem> m_insertItems;

        robot2D::Key m_shortCutKey{robot2D::Key::Q};


        std::string m_playloadIdentifier = "TreeNodeItem";
    };


}