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

#include "robot2D/Core/Vector2.hpp"
#include "Transformable.hpp"
#include "Drawable.hpp"
#include "Texture.hpp"
#include "Rect.hpp"
#include "Color.hpp"

namespace robot2D {

    /**
     * \brief in 2D worlds Sprite class is flyweight to wrap texture and make it Drawable on screen and
     * Have different Transformations. Core feature, which you will use to alive us apps
     */
    class Sprite: public Transformable, public Drawable{
    public:
        Sprite();
        ~Sprite()override = default;

        /// \brief set custom color to your sprite
        void setColor(const Color& color);
        Color& getColor();

        /// \brief allow to set Size/Scale of object
        void setScale(const vec2f &factor) override;

        /// \brief set texture data to our Sprite flyweight
        void setTexture(const Texture& );
        const Texture* getTexture();

        /// \brief return to you rect in world coordinates
        FloatRect getGlobalBounds() const;

        /// \brief return to you rect in local Sprite coordinates
        FloatRect getLocalBounds() const;
    protected:
        void draw(RenderTarget &target, RenderStates states) const override;

    protected:
        const Texture* m_texture;
        IntRect m_texture_rect;
        Color m_color;
    };
}