/*********************************************************************
(c) Alex Raag 2024
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

#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Graphics/Texture.hpp>

namespace editor {
    class Quad: public robot2D::Drawable {
    public:
        Quad() = default;
        Quad(const Quad& other) = default;
        Quad& operator=(const Quad& other) = default;
        Quad(Quad&& other) = default;
        Quad& operator=(Quad&& other) = default;
        ~Quad() = default;

        bool contains(const robot2D::vec2f& point) const {
            return getFrame().contains(point);
        }

        bool contains(float x, float y) const {
            return contains({ x, y });
        }

        [[nodiscard]]
        robot2D::FloatRect getFrame() const;

        void draw(robot2D::RenderTarget& target,
                  robot2D::RenderStates states) const override;

    // private:
        robot2D::vec2f position { };
        robot2D::vec2f size { };
        robot2D::Color color { };
        robot2D::Texture* texture{nullptr};
    };
}