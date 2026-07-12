/*********************************************************************
(c) Alex Raag 2026
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

#include <cmath>
#include <editor/QuadTree.hpp>

namespace editor {

    robot2D::vec2f rotatePoint(float angle, robot2D::vec2f point, robot2D::vec2f center_of_rotation) {
        auto convertAngle = [](float degrees) {
            return degrees * (M_PI / 180);
        };

        float sinus = std::sin(-convertAngle(angle));
        float cosinus = std::cos(-convertAngle(angle));

        robot2D::vec2f tmp;
        point = point - center_of_rotation;
        tmp.x = point.x * cosinus - point.y * sinus;
        tmp.y = point.x * sinus + point.y * cosinus;
        point = tmp + center_of_rotation;
        return point;
    }

} /// namespace editor