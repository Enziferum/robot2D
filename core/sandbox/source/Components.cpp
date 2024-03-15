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

#include <sandbox/Components.hpp>

SpriteComponent::SpriteComponent(): m_color(robot2D::Color::White) {

}

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

TransformComponent::TransformComponent() {}

void TransformComponent::setPosition(const robot2D::vec2f& pos) {
    for(auto child: m_children) {
        auto& transform = child.getComponent<TransformComponent>();
        robot2D::vec2f offset = m_pos - child.getComponent<TransformComponent>().getPosition();
        transform.setPosition(pos - offset);
    }
    Transformable::setPosition(pos);
}

void TransformComponent::addChild(robot2D::ecs::Entity entity) {
    auto found = std::find_if(m_children.begin(), m_children.end(), [&entity](robot2D::ecs::Entity ent) {
        return entity == ent;
    });
    if(found == m_children.end())
        m_children.emplace_back(entity);
}

void TransformComponent::removeChild(robot2D::ecs::Entity entity) {
    m_children.erase(std::remove_if(m_children.begin(), m_children.end(), [&entity](robot2D::ecs::Entity ent) {
        return entity == ent;
    }), m_children.end());
}

bool TransformComponent::hasChildren() const {
    return !m_children.empty();
}
