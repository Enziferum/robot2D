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

#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Graphics/Sprite.hpp>

namespace robot2D {

    Sprite::Sprite(): m_texture(nullptr),
    m_color(Color::White){

    }

    void Sprite::setTexture(const Texture& texture) {
        if(!m_texture && (m_texture_rect == IntRect())){
            auto size = texture.get_size();
            //it's not correct for us
            m_texture_rect = IntRect(0, 0, size.x, size.y);
            setSize(size.x, size.y);
        }
        m_texture = &texture;
    }


    const Texture* Sprite::getTexture() {
        return m_texture;
    }

    FloatRect Sprite::getLocalBounds() const {
        float w = std::abs(m_size.x);
        float h = std::abs(m_size.y);

        return FloatRect(0.f, 0.f, w, h);
    }

    FloatRect Sprite::getGlobalBounds() const {;
        return getTransform().transformRect(getLocalBounds());
    }

    void Sprite::setScale(const vec2f& factor) {
        m_texture_rect = IntRect (0, 0, factor.x, factor.y);
        if (factor.x < m_size.x && factor.y < m_size.y)
            Transformable::setScale(robot2D::vec2f(factor.x / m_size.x,
                                                   factor.y / m_size.y));

    }

    void Sprite::setColor(const Color& color) {
        m_color = color;
    }

    Color &Sprite::getColor() {
        return m_color;
    }

    void Sprite::draw(RenderTarget& target, RenderStates states) const {
        if(m_texture){
            auto t = getTransform();
            t = t.scale(m_size);
            states.transform *= t;
            states.texture = m_texture;
            states.color = m_color;
            target.draw(states);
        }
    }

}