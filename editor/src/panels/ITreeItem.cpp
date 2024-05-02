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

#include <editor/panels/ITreeItem.hpp>

namespace editor {

    ITreeItem::ITreeItem(): m_id() {}
    ITreeItem::ITreeItem(UUID id) : m_id{ id } {}
    ITreeItem::~ITreeItem() = default;

    void ITreeItem::setName(std::string* name) {
        m_name = name;
    }

    void ITreeItem::setTexture(robot2D::Texture& texture, const robot2D::Color& tintColor) {
        m_iconTexture = &texture;
        m_tintColor = tintColor;
    }

    bool ITreeItem::deleteChild(ITreeItem::Ptr item) {
        m_deletePendingItems.emplace_back(item);
        return true;
    }

    void ITreeItem::addChild(ITreeItem::Ptr child) {
        child -> m_parent = this;
        child -> isQueryDeletion = true;
        m_childrens.emplace_back(child);
    }

    void ITreeItem::removeSelf() {
        if (!isChild() || !m_parent)
            return;
        m_parent->removeChild(this);
    }

    void ITreeItem::update() {
        for (auto delItem : m_deletePendingItems) 
            m_childrens.erase(std::remove_if(m_childrens.begin(), m_childrens.end(),
                                             [&delItem](auto item) {
                return *delItem == *item;
            }), m_childrens.end());
        m_deletePendingItems.clear();
    }

    void ITreeItem::removeChild(ITreeItem* child) {
        auto found = std::find_if(m_childrens.begin(),
            m_childrens.end(), [&child](ITreeItem::Ptr item) {
                return *child == *item;
            });

        m_deletePendingItems.emplace_back(*found);
    }

    /// -
    ///  - -
    ///  -- -

    const std::size_t ITreeItem::getChildValue(std::size_t& size) const {
        if(hasChildrens()) {
            for(auto& child: m_childrens)
                size += child -> getChildValue(size);
            return size;
        }
        size += m_childrens.size();
        return size;
    }
}