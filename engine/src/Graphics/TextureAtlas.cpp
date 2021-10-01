//
// Created by Necromant on 02.10.2021.
//

#include <robot2D/Graphics/TextureAtlas.hpp>


namespace robot2D {

    TextureAtlas::TextureAtlas(): m_texture(), m_ItemSize(), m_itemsCount(0) {

    }

    bool TextureAtlas::loadFromFile(const std::string& file, const vec2u& itemSize, const vec2f& offset) {
        if(!m_texture.loadFromFile(file))
            return false;
        m_ItemSize = itemSize;
        auto texSize = m_texture.getSize();

        return true;
    }

    const Texture& TextureAtlas::getTexture() const {
        return m_texture;
    }

    const robot2D::vec2u &TextureAtlas::getItemSize() const {
        return m_ItemSize;
    }

    const robot2D::FloatRect& TextureAtlas::getItemFrame(const vec2u& index) const {

        return {};
    }

    const size_t& TextureAtlas::getItemsCount() const {
        return m_itemsCount;
    }
}