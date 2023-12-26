#pragma once
#include <vector>
#include <string>
#include <memory>
#include <tuple>

#include <robot2D/Core/Keyboard.hpp>

namespace robot2D {
    constexpr int NO_INDEX = -1;

    class ITreeItem {
    public:
        using Ptr = std::shared_ptr<ITreeItem>;

        ITreeItem(): m_id(-1){}
        ITreeItem(int id): m_id{id}{}
        virtual ~ITreeItem() = 0;

        bool hasChildrens() const { return !m_childrens.empty(); }

        int getID() const { return m_id; }

        void setName(std::string& name) {
            m_name = &name;
        }

        /// \brief don't call if user data was set void or you want to call on void type
        template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, void>>>
        T* getUserData() {
            return static_cast<T*>(getUserDataInternal());
        }

        bool isChild() const { return m_child_id != NO_INDEX; }

        bool deleteChild(ITreeItem::Ptr item) {
            m_deletePendingItems.emplace_back(item);
            return true;
        }

        void addChild(ITreeItem::Ptr child) {
            child -> m_parent = this;
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
                return child == item.get();
            });
            m_childrens.erase(found, m_childrens.end());
        }

        virtual void* getUserDataInternal() = 0;
        int m_id{NO_INDEX};
        int m_child_id{NO_INDEX};
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
            child -> m_child_id = m_childrens.size();
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
        void* getUserDataInternal() override {
            assert(m_userData != nullptr && "before get data store it, using SetUserData Method");
            return static_cast<void*>(m_userData.get());
        }
    private:
        /// TODO(a.raag): add UserDataStorage
        TreeItemUserData m_userData{nullptr};
        int m_childIndexOffset = 100;
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

        template<typename T, typename ... Args>
        typename TreeItem<T>::Ptr addItem(Args&& ... args) {
            static_assert(std::is_standard_layout_v<T> && "UserData Type must be pod");
            auto treeItem = std::make_shared<TreeItem<T>>(std::forward<Args>(args)...);
            if(!treeItem) {
                /// error
            }
            treeItem -> m_id = m_items.size();
            m_items.template emplace_back(treeItem);
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

        template<typename T>
        TreeItem<T>* getItem(int index) {
            return dynamic_cast<TreeItem<T>*>(m_items[index].get());
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

        void render();
    private:
        ITreeItem::Ptr findByID(int ID);

    private:
        std::string m_name;
        std::vector<ITreeItem::Ptr> m_items{};
        std::vector<ITreeItem::Ptr> m_deletePendingItems{};

        enum class ReorderDeleteType {
            First, Last
        };

        using Iterator = std::vector<ITreeItem::Ptr>::iterator;
        using InsertItem = std::tuple<Iterator, ITreeItem::Ptr, ReorderDeleteType>;

        int m_tree_base_flags = 0;
        int m_selectedID = NO_INDEX;
        int m_childSelectedID = NO_INDEX;

        ItemCallback m_selectCallback;
        ItemCallback m_callback;
        ReorderItemCallback m_reorderCallback;
        ReorderItemCallback m_makeAsChildCallback;
        ReorderItemCallback m_removeAsChildCallback;

        std::vector<InsertItem> m_insertItems;

        robot2D::Key m_shortCutKey{robot2D::Key::Q};


        std::string m_playloadIdentifier = "TreeNodeItem";
    };


}