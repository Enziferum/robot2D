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

#include <editor/Components.hpp>

namespace editor {
    SpriteComponent::SpriteComponent():
    m_color(robot2D::Color::White),
    m_texture(nullptr) {}

    void SpriteComponent::setTexture(const robot2D::Texture &texture) {
        m_texture = &texture;
    }

    robot2D::Texture& SpriteComponent::getTexture() {
        return const_cast<robot2D::Texture &>(*m_texture);
    }

    const robot2D::Texture& SpriteComponent::getTexture() const {
        return *m_texture;
    }

    void SpriteComponent::setColor(const robot2D::Color& color) {
        m_color = color;
    }

    const robot2D::Color& SpriteComponent::getColor() const {
        return m_color;
    }


    CameraComponent::CameraComponent(): m_view(), m_actualViewport() {}

    void CameraComponent::setViewport(const robot2D::FloatRect& viewport) {
        m_actualViewport = viewport;
        m_view.reset(m_actualViewport);
    }

    float CameraComponent::getZoom() const {
        auto actualSize = m_view.getSize();
        robot2D::vec2f defaultSize = { m_actualViewport.width, m_actualViewport.height};
        robot2D::vec2f zoom = {defaultSize.x / actualSize.x, defaultSize.y / actualSize.y};
        return zoom.x;
    }

}