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

#include <cmath>
#include "robot2D/Graphics/Transform.h"

namespace robot2D{
    Transform::Transform() {
        m_mat[0] = 1.f;m_mat[4] = 0.f;m_mat[8] = 0.f;m_mat[12] = 0.f;
        m_mat[1] = 0.f;m_mat[5] = 1.f;m_mat[9] = 0.f;m_mat[13] = 0.f;
        m_mat[2] = 0.f;m_mat[6] = 0.f;m_mat[10] = 1.f;m_mat[14] = 0.f;
        m_mat[3] = 0.f;m_mat[7] = 0.f;m_mat[11] = 0.f;m_mat[15] = 1.f;
    }

    Transform::Transform(float a00, float a01, float a02,
                         float a10, float a11, float a12,
                         float a20, float a21, float a22) {

        m_mat[0] = a00;m_mat[4] = a01;m_mat[8] = 0.f;m_mat[12] = a02;
        m_mat[1] = a10;m_mat[5] = a11;m_mat[9] = 0.f;m_mat[13] = a12;
        m_mat[2] = 0.f;m_mat[6] = 0.f;m_mat[10] = 1.f;m_mat[14] = 0.f;
        m_mat[3] = a20;m_mat[7] = a21;m_mat[11] = 0.f;m_mat[15] = a22;
    }

    vec2f Transform::transformPoint(float x, float y) const
    {
        return vec2f(m_mat[0] * x + m_mat[4] * y + m_mat[12],
                     m_mat[1] * x + m_mat[5] * y + m_mat[13]);
    }


////////////////////////////////////////////////////////////
    vec2f Transform::transformPoint(const vec2f & point) const
    {
        return transformPoint(point.x, point.y);
    }


////////////////////////////////////////////////////////////
    FloatRect Transform::transformRect(const FloatRect& rectangle) const
    {
        // Transform the 4 corners of the rectangle
        const vec2f points[] =
                {
                        transformPoint(rectangle.lx, rectangle.ly),
                        transformPoint(rectangle.lx, rectangle.ly + rectangle.height),
                        transformPoint(rectangle.lx + rectangle.width, rectangle.ly),
                        transformPoint(rectangle.lx + rectangle.width, rectangle.ly + rectangle.height)
                };

        // Compute the bounding rectangle of the transformed points
        float left = points[0].x;
        float top = points[0].y;
        float right = points[0].x;
        float bottom = points[0].y;
        for (int i = 1; i < 4; ++i)
        {
            if      (points[i].x < left) left = points[i].x;
            else if (points[i].x > right) right = points[i].x;
            if      (points[i].y < top) top = points[i].y;
            else if (points[i].y > bottom) bottom = points[i].y;
        }

        return FloatRect(left, top, right - left, bottom - top);
    }

    Transform& Transform::combine(const Transform& other) {
        const float* a = m_mat;
        const float* b = other.get_matrix();
        *this = Transform(a[0] * b[0]  + a[4] * b[1]  + a[12] * b[3],
                          a[0] * b[4]  + a[4] * b[5]  + a[12] * b[7],
                          a[0] * b[12] + a[4] * b[13] + a[12] * b[15],
                          a[1] * b[0]  + a[5] * b[1]  + a[13] * b[3],
                          a[1] * b[4]  + a[5] * b[5]  + a[13] * b[7],
                          a[1] * b[12] + a[5] * b[13] + a[13] * b[15],
                          a[3] * b[0]  + a[7] * b[1]  + a[15] * b[3],
                          a[3] * b[4]  + a[7] * b[5]  + a[15] * b[7],
                          a[3] * b[12] + a[7] * b[13] + a[15] * b[15]);

        return *this;
    }

    bool operator==(const Transform& lhs, const Transform& rhs){
        const float* a = lhs.get_matrix();
        const float* b = rhs.get_matrix();

        return ((a[0]  == b[0])  && (a[1]  == b[1])  && (a[3]  == b[3]) &&
                (a[4]  == b[4])  && (a[5]  == b[5])  && (a[7]  == b[7]) &&
                (a[12] == b[12]) && (a[13] == b[13]) && (a[15] == b[15]));
    }

    bool operator !=(const Transform& lhs, const Transform& rhs){
        return !(lhs == rhs);
    }

    const float* Transform::get_matrix() const {
        return m_mat;
    }

    Transform& Transform::translate(float x, float y) {
        Transform translation(1, 0, x,
                              0, 1, y,
                              0, 0, 1);

        return combine(translation);
    }

    Transform& Transform::translate(const vec2f &offset) {
        return translate(offset.x, offset.y);
    }

    Transform& Transform::rotate(float angle) {
        float rad = angle * 3.141592654f / 180.f;
        float cose = std::cos(rad);
        float sine = std::sin(rad);
        Transform transform(cose, -sine, 0,
                            sine, cose, 0,
                            0, 0, 1);

        return combine(transform);
    }

    Transform& Transform::rotate_around(float angle, vec2f point) {
        float rad = angle * 3.141592654f / 180.f;
        float cose = std::cos(rad);
        float sine = std::sin(rad);
        Transform transform(cose, -sine, point.x * (1 - cose) + point.y * sine,
                            sine, cose, point.y * (1 - cose) - point.x * sine,
                            0, 0, 1);

        return combine(transform);
    }

    Transform& Transform::scale(float x, float y) {
        Transform scaling(x, 0, 0,
                          0, y, 0,
                          0,0,1);
        return combine(scaling);
    }

    Transform& Transform::scale(const vec2f &factor) {
        return scale(factor.x, factor.y);
    }


    Transform operator *(const Transform& left, const Transform& right)
    {
        return Transform(left).combine(right);
    }


////////////////////////////////////////////////////////////
    Transform& operator *=(Transform& left, const Transform& right)
    {
        return left.combine(right);
    }


////////////////////////////////////////////////////////////
    vec2f operator *(const Transform& left, const vec2f & right)
    {
        return left.transformPoint(right);
    }

}