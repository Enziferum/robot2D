#pragma once

#include <deque>
#include <array>
#include <memory>
#include <list>
#include <cmath>

#include <robot2D/Graphics/Rect.hpp>


namespace editor {

    template<typename T>
    struct QuadTreeItemLocation {
        using ValuePair = std::pair<robot2D::FloatRect, T>;
        typename std::list<ValuePair>* container;
        typename std::list<ValuePair>::iterator iterator;
    };

    template<typename T>
    struct QuadTreeItem {
        using Container = std::list<QuadTreeItem<T>>;
        using ContainerIterator = typename Container::iterator;

        T value;
        QuadTreeItemLocation<ContainerIterator> iterator;
    };


    robot2D::vec2f rotatePoint(float angle, robot2D::vec2f point, robot2D::vec2f center_of_rotation);

    template<typename T>
    class QuadTree {
    private:
        using Ptr = std::shared_ptr<QuadTree<T>>;
        using ValuePair = std::pair<robot2D::FloatRect, T>;
    public:
        QuadTree() = default;
        QuadTree(const robot2D::FloatRect& region, std::size_t depth);
        QuadTree(const QuadTree& other) = delete;
        QuadTree& operator=(const QuadTree& other) = delete;
        QuadTree(QuadTree&& other) = delete;
        QuadTree& operator=(QuadTree&& other) = delete;
        ~QuadTree() = default;

        void resize(const robot2D::FloatRect& newArea);

        QuadTreeItemLocation<T> insert(const T& object, const robot2D::FloatRect& objectRect);
        std::list<T> search(const robot2D::FloatRect& rect) const;
        void search(const robot2D::FloatRect& rect, std::list<T>& items) const;
        void search(const robot2D::vec2f& point, std::list<T>& items) const;


        void items(std::list<T>& listItems) const
        {
            // No questions asked, just return child items
            for (const auto& p : m_objects)
                listItems.push_back(p.second);

            // Now add children of this layer's items
            for (int i = 0; i < 4; i++)
                if (m_childrenTrees[i])
                    m_childrenTrees[i] -> items(listItems);
        }


        std::list<T> items() const
        {
            // No questions asked, just return child items
            std::list<T> listItems;
            items(listItems);
            return listItems;
        }

        void clearAll();
    private:
        inline static constexpr std::size_t maxDepth = 8;

        std::size_t m_depth = 0;

        robot2D::FloatRect m_rect;

        std::array<robot2D::FloatRect, 4> m_childrenRects {};
        std::array<Ptr, 4> m_childrenTrees {};

        std::list<ValuePair> m_objects;
    };

    template<typename T>
    QuadTree<T>::QuadTree(const robot2D::FloatRect& region, std::size_t depth): m_rect{region}, m_depth(depth) {}

    template<typename T>
    void QuadTree<T>::resize(const robot2D::FloatRect& newArea) {
        clearAll();
        m_rect = newArea;

        robot2D::vec2f childSize = { m_rect.width / 2.f, m_rect.height / 2.f };

        m_childrenRects = {
            robot2D::FloatRect { m_rect.topPoint(), childSize },
            robot2D::FloatRect {{ m_rect.lx + childSize.x, m_rect.ly  }, childSize },
            robot2D::FloatRect {{ m_rect.lx , m_rect.ly + childSize.y }, childSize },
            robot2D::FloatRect { { m_rect.lx + childSize.x, m_rect.ly + childSize.y }, childSize }
        };

    }

    template<typename T>
    QuadTreeItemLocation<T> QuadTree<T>::insert(const T& object, const robot2D::FloatRect& objectRect) {
        for(int i = 0 ; i < 4; ++i) {
            if(m_childrenRects[i].contains(objectRect)) {
                if(m_depth + 1 < maxDepth) {
                    if(!m_childrenTrees[i])
                        m_childrenTrees[i] = std::make_shared<QuadTree<T>>(m_childrenRects[i], m_depth + 1);

                    return m_childrenTrees[i] -> insert(object, objectRect);
                }
            }
        }

        m_objects.push_back( std::make_pair( objectRect, object ) );
        return { &m_objects, std::prev(m_objects.end()) };
    }


    template<typename T>
    std::list<T> QuadTree<T>::search(const robot2D::FloatRect& rect) const{
        std::list<T> listItems;
        search(rect, listItems);
        return listItems;
    }

    template<typename T>
    void QuadTree<T>::search(const robot2D::FloatRect& rect, std::list<T>& items) const {
        // First, check for items belonging to this area, add them to the list
        // if there is overlap
        for (const auto& p : m_objects)
        {
            if (rect.intersects(p.first))
                items.push_back(p.second);
        }

        // Second, recurse through children and see if they can
        // add to the list
        for (int i = 0; i < 4; i++)
        {
            if (m_childrenTrees[i])
            {
                // If child is entirely contained within area, recursively
                // add all of its children, no need to check boundaries
                if (rect.contains(m_childrenRects[i]))
                    m_childrenTrees[i] -> items(items);

                    // If child overlaps with search area then checks need
                    // to be made
                else if (m_childrenRects[i].intersects(rect))
                    m_childrenTrees[i] -> search(rect, items);
            }
        }
    }

    template<typename T>
    void QuadTree<T>::search(const robot2D::vec2f& point, std::list<T>& items) const {
        constexpr auto getRect = [](const robot2D::vec2f& point) {
            return robot2D::FloatRect{point.x, point.y, 1, 1};
        };

        for (const auto& p : m_objects)
        {
            const auto& rect = p.first;
            if(!rect.isRotated()) {
                auto&& pointRect = getRect(point);
                if (pointRect.intersects(rect))
                    items.push_back(p.second);
            }
            else {
                auto pointRect = getRect(rotatePoint(rect.getRotateAngle(), point,
                                                       rect.centerPoint()));
                if (pointRect.intersects(rect))
                    items.push_back(p.second);
            }
        }

        // Second, recurse through children and see if they can
        // add to the list
        for (int i = 0; i < 4; i++)
        {
            if (m_childrenTrees[i])
            {
                // If child is entirely contained within area, recursively
                // add all of its children, no need to check boundaries
                const auto& rect = m_childrenRects[i];
                if(!rect.isRotated()) {
                    auto&& pointRect = getRect(point);
                    if (pointRect.contains(rect))
                        m_childrenTrees[i] -> items(items);
                    else if(rect.intersects(pointRect))
                        m_childrenTrees[i] -> search(point, items);
                }
                else {
                    auto pointRect = getRect(rotatePoint(rect.getRotateAngle(), point,
                                                           rect.centerPoint()));
                    if (pointRect.contains(rect))
                        m_childrenTrees[i] -> items(items);
                    else if(rect.intersects(pointRect))
                        m_childrenTrees[i] -> search(point, items);
                }
            }
        }
    }


    template<typename T>
    void QuadTree<T>::clearAll() {
        m_objects.clear();
        for(auto& child: m_childrenTrees) {
            if(!child)
                continue;
            child -> clearAll();
            child.reset();
        }
    }



    template<typename T>
    class QuadTreeContainer {
    private:
        using Item = QuadTreeItem<T>;
        using Container = std::list<Item>;
    public:
        using ContainerIterator = typename std::list<Item>::iterator;

        QuadTreeContainer() = default;
        QuadTreeContainer(const QuadTreeContainer& other) = delete;
        QuadTreeContainer& operator=(const QuadTreeContainer& other) = delete;
        QuadTreeContainer(QuadTreeContainer&& other) = delete;
        QuadTreeContainer& operator=(QuadTreeContainer&& other) = delete;
        ~QuadTreeContainer() = default;


        bool empty() const { return m_container.empty(); }
        std::size_t size() const { m_container.size(); }

        void resizeRectangle(const robot2D::FloatRect& newArea) {
            m_tree.resize(newArea);
        }

        void clear() {
            m_tree.clearAll();
            m_container.clear();
        }

        ContainerIterator begin() const { return m_container.begin(); }
        ContainerIterator end() const { { return m_container.begin(); }}
        typename Container::const_iterator cbegin() const { return m_container.cbegin(); }
        typename Container::const_iterator cend() const { return m_container.cend(); }


        ContainerIterator insert(const T& value, const robot2D::FloatRect& rect) {
            Item item;
            item.value = value;
            m_container.push_back(item);
            auto& backItem = m_container.back();
            auto lastIterator = std::prev(m_container.end());
            backItem.iterator = m_tree.insert(lastIterator, rect);
            return lastIterator;
        }

        std::list<ContainerIterator> search(const robot2D::FloatRect& rect) const
        {
            std::list<ContainerIterator> searchItems;
            m_tree.search(rect, searchItems);
            return searchItems;
        }

        std::list<ContainerIterator> search(const robot2D::vec2f& point) const
        {
            std::list<ContainerIterator> searchItems;
            m_tree.search(point, searchItems);
            return searchItems;
        }

        bool remove(ContainerIterator& item) {
            auto* container = item -> iterator.container;
            if(!container)
                return false;
            container -> erase(item -> iterator.iterator);
            m_container.erase(item);
            return true;
        }

        void relocate(ContainerIterator& item, const robot2D::FloatRect& newRect) {
            auto* container = item -> iterator.container;
            if(!container)
                return;
            container -> erase(item -> iterator.iterator);

            item -> iterator = m_tree.insert(item, newRect);
        }
    private:
        Container m_container;
        QuadTree<ContainerIterator> m_tree;
    };


}