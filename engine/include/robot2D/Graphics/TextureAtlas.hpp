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

#pragma once
#include "Rect.hpp"
#include "Texture.hpp"

namespace robot2D {
    /**
     * \brief Packed several textures into one.
     * \details To optimize rendering stuff more simple to work with one texture. \n
     * Allow to load texture and get packed texture with their frames.
     */
    class ROBOT2D_EXPORT_API TextureAtlas {
    public:
        TextureAtlas();
        ~TextureAtlas() = default;

        /// Load texture and fill information about packed textures
        bool loadFromFile(const std::string& file, const robot2D::vec2u& itemSize,
                          const robot2D::vec2f& offset = {});

        /// How many textures are packed
        const size_t& getItemsCount() const;
        /// Whole texture
        const Texture& getTexture() const;
        /// Size of one packed texture
        const robot2D::vec2u& getItemSize() const;
        /// Frame = Rectangle of packed texture with offsets from topleft coordinate
        robot2D::FloatRect getItemFrame(const robot2D::vec2u& index) const;
    private:
        Texture m_texture;
        robot2D::vec2u m_ItemSize;
        size_t m_itemsCount;
    };
}
