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
#include <limits>

#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Color.hpp>
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

        void setName(std::string* name) {
            m_name = name;
        }

        void setTexture(robot2D::Texture& texture, const robot2D::Color& tintColor) {
            m_iconTexture = &texture;
            m_tintColor = tintColor;
        }

        /// \brief don't call if user data was set void or you want to call on void type
        template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, void>>>
        T* getUserData() const {
            return static_cast<T*>(getUserDataInternal());
        }

        bool isChild() const { return m_parent != nullptr; }

        bool deleteChild(ITreeItem::Ptr item) {
            m_deletePendingItems.emplace_back(item);
            return true;
        }

        void addChild(ITreeItem::Ptr child) {
            child -> m_parent = this;
            child -> isQueryDeletion = true;
            m_childrens.emplace_back(child);
        }

        void removeSelf() {
            if(!isChild() || !m_parent)
                return;
            m_parent -> removeChild(this);
        }

        void update() {
            for(auto delItem: m_deletePendingItems)
                m_childrens.erase(std::remove_if(m_childrens.begin(), m_childrens.end(), [&delItem](auto item) {
                    return delItem -> m_id == item -> m_id;
            }), m_childrens.end());
        }

        std::vector<ITreeItem::Ptr>& getChildrens() { return m_childrens; }
    protected:
        void removeChild(ITreeItem* child) {
            auto found = std::find_if(m_childrens.begin(),
                                      m_childrens.end(), [&child](ITreeItem::Ptr item) {
                /// TODO(a.raag): add operator==();
                return child -> m_id == item -> m_id;
            });

            m_deletePendingItems.emplace_back(*found);
        }

        virtual void* getUserDataInternal() const = 0;
        UUID m_id;

        std::string* m_name{nullptr};
        ITreeItem* m_parent{nullptr};
        std::vector<ITreeItem::Ptr> m_childrens;
        std::vector<ITreeItem::Ptr> m_deletePendingItems{};

        friend class TreeHierarchy;
        robot2D::Texture* m_iconTexture{nullptr};
        robot2D::Color m_tintColor = robot2D::Color::White;

        bool isQueryDeletion = false;
    };
}