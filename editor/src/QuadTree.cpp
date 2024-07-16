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