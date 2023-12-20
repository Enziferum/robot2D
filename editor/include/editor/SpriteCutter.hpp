/*********************************************************************
(c) Alex Raag 2023
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

#include <vector>
#include <set>

#include "robot2D/Graphics/Texture.hpp"
#include "robot2D/Graphics/Rect.hpp"
#include "robot2D/Graphics/Color.hpp"

namespace editor {
    struct ColorPoint {
        bool opaque;
        robot2D::vec2i pos;

        explicit operator bool() const noexcept {
            return opaque;
        }
    };

    using visitMatrix = std::vector<std::vector<int>>;
    using colorPointMatrix = std::vector<std::vector<ColorPoint>>;

    class SpriteCutter {
    public:
        using colorPoint = std::pair<robot2D::Color, robot2D::vec2i>;
        SpriteCutter() = default;
        ~SpriteCutter() = default;

        std::vector<robot2D::IntRect> cutFrames(const robot2D::UIntRect& clipRegion, robot2D::Image& image,
                                                robot2D::vec2f worldPosition);
    private:
        bool isSafe(const colorPointMatrix& M, int row, int col, visitMatrix& visited);
        void dfs(const colorPointMatrix& M, int row, int col, visitMatrix& visited,
                 std::vector<robot2D::vec2i>& points);
    };

} // namespace editor