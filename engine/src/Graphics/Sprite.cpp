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

    Sprite::Sprite():
    m_texture(nullptr),
    m_color(Color::White) {}

    void Sprite::setTexture(const Texture& texture, const IntRect& textureRect) {
        m_textureRect = textureRect;
        m_texture = &texture;
        auto size = texture.getSize();
      //  setSize(size.as<float>());

        if(!m_texture && (textureRect == IntRect())){
            m_textureRect = IntRect(0, 0, size.x, size.y);
        }

        setTextureRect(m_textureRect);
    }

    const Texture* Sprite::getTexture() {
        return m_texture;
    }

    FloatRect Sprite::getLocalBounds() const {
        float w = std::abs(m_size.x);
        float h = std::abs(m_size.y);
        return {0.f, 0.f, w, h};
    }

    FloatRect Sprite::getGlobalBounds() const {;
        return getTransform().transformRect(getLocalBounds());
    }

    void Sprite::setScale(const vec2f& factor) {
        m_textureRect = IntRect (0, 0, factor.x, factor.y);
        if (factor.x < m_size.x && factor.y < m_size.y)
            Transformable::setScale(robot2D::vec2f(factor.x / m_size.x,
                                                   factor.y / m_size.y));

    }

    void Sprite::setColor(const Color& color) {
        m_color = color;
    }

    Color& Sprite::getColor() {
        return m_color;
    }

    const Color& Sprite::getColor() const {
        return m_color;
    }

    void Sprite::draw(RenderTarget& target, RenderStates states) const {
        if(!m_texture)
            return;
        auto t = getTransform();
        if(m_size != robot2D::vec2f{})
            t = t.scale(m_size);

        states.transform *= t;
        states.texture = m_texture;
        states.color = m_color;

        vertices[0].position = states.transform * robot2D::vec2f {0.F, 0.F};
        vertices[1].position = states.transform * robot2D::vec2f {1.F, 0.F};
        vertices[2].position = states.transform * robot2D::vec2f {1.F, 1.F};
        vertices[3].position = states.transform * robot2D::vec2f {0.F, 1.F};

        target.draw( {vertices[0], vertices[1], vertices[2], vertices[3] }, states);
    }

    void Sprite::setTextureRect(const IntRect& textureRect) {
        m_textureRect = textureRect;

        float left   = static_cast<float>(m_textureRect.lx);
        float right  = left + m_textureRect.width;

        float bottom = m_textureRect.ly;
        float top = m_textureRect.ly + m_textureRect.height;

        auto convertToGL = [](float value, float textureSize) {
            return static_cast<float>(value / textureSize);
        };

        auto tx_s = m_texture -> getSize();
        vertices[0].texCoords = {convertToGL(left, static_cast<float>(tx_s.x)),
                                 convertToGL(bottom, static_cast<float>(tx_s.y))};
        vertices[1].texCoords = {convertToGL(right, static_cast<float>(tx_s.x)),
                                 convertToGL(bottom, static_cast<float>(tx_s.y))};
        vertices[2].texCoords = {convertToGL(right, static_cast<float>(tx_s.x)),
                                 convertToGL(top, static_cast<float>(tx_s.y))};
        vertices[3].texCoords = {convertToGL(left, static_cast<float>(tx_s.x)),
                                 convertToGL(top, static_cast<float>(tx_s.y))};

    }

    const IntRect& Sprite::getTextureRect() const {
        return m_textureRect;
    }

}