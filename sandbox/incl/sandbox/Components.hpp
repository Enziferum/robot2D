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

#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Graphics/Transformable.hpp>

class SpriteComponent final {
public:
    SpriteComponent();
    ~SpriteComponent() = default;

    void setTexture(const robot2D::Texture& texture);
    robot2D::Texture& getTexture();
    const robot2D::Texture& getTexture() const;


    void setColor(const robot2D::Color& color);
    const robot2D::Color& getColor() const;
private:
    const robot2D::Texture* m_texture;
    robot2D::Color m_color;
};


class TransformComponent final: public robot2D::Transformable {
public:
    TransformComponent();
    ~TransformComponent() override = default;

};
