//
// Created by Necromant on 02.10.2021.
//

#pragma once
#include "Rect.hpp"
#include "Texture.hpp"

namespace robot2D {
    class TextureAtlas {
    public:
        TextureAtlas();
        ~TextureAtlas() = default;

        bool loadFromFile(const std::string& file, const robot2D::vec2u& itemSize,
                          const robot2D::vec2f& offset = {});

        const size_t& getItemsCount() const;
        const Texture& getTexture() const;
        const robot2D::vec2u& getItemSize() const;
        const robot2D::FloatRect& getItemFrame(const robot2D::vec2u& index) const;
    private:
        Texture m_texture;
        robot2D::vec2u m_ItemSize;
        size_t m_itemsCount;
    };
}
