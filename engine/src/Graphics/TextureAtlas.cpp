/*********************************************************************
(c) Alex Raag 2021
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