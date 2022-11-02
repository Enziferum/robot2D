/*********************************************************************
(c) Alex Raag 2022
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
#include <robot2D/Graphics/Math3D.hpp>

namespace robot2D {
    Quaternion::Quaternion(): x{0}, y{0}, z{0}, w{0} {}
    Quaternion::Quaternion(float x, float y, float z, float w): x{x}, y{y}, z{z}, w{w} {}

    Quaternion::Quaternion(robot2D::vec3f eulerAngle) {
        robot2D::vec3f c = cosVec3F(eulerAngle * 0.5F);
        robot2D::vec3f s = sinVec3F(eulerAngle * 0.5F);

        x = s.x * c.y * c.z - c.x * s.y * s.z;
        y = c.x * s.y * c.z + s.x * c.y * s.z;
        z = c.x * c.y * s.z - s.x * s.y * c.z;
        w = c.x * c.y * c.z + s.x * s.y * s.z;
    }

    Quaternion Quaternion::Conjugate() const {
        return {-x, -y, -z, -w};
    }

    vec3f operator *(const Quaternion& q, const vec3f& v) {
        vec3f const QuatVector(q.x, q.y, q.z);
        vec3f const uv(cross(QuatVector, v));
        vec3f const uuv(cross(QuatVector, uv));
        return v + ((uv * q.w) + uuv) * static_cast<float>(2);
    }

    vec3f cosVec3F(const vec3f& in) {
        return {std::cos(in.x), std::cos(in.y), std::cos(in.z)};
    }

    vec3f sinVec3F(const vec3f& in) {
        return {std::sin(in.x), std::sin(in.y), std::sin(in.z)};
    }

    vec3f cross(const vec3f& left, const vec3f& right) {
        return {
                left.y * right.z - left.z * right.y,
                left.z * right.x - left.x * right.z,
                left.x * right.y - left.y * right.x
        };
    }

    vec3f rotate(quat const& quat, const vec3f& axis) {
        return quat * axis;
    }

    mat4 projectionPerspective(float fov, float aspectRatio, float zNear, float zFar) {
        const float tanHalfFovy = std::tan(fov / 2.f);
        const float frustum_length = zFar - zNear;

        mat4 matrix{0.F};
        matrix[0] = 1.f / (tanHalfFovy * aspectRatio);
        matrix[5] = 1.f / tanHalfFovy;
        matrix[10] = -((zFar + zNear) / frustum_length);
        matrix[11] = -1.f;
        matrix[14] = -((2.F * zNear * zFar) / frustum_length);
        return matrix;
    }

    mat4 toMat4(const quat& q) {
        using T = float;
        robot2D::mat4 matrix;

        T qxx(q.x * q.x);
        T qyy(q.y * q.y);
        T qzz(q.z * q.z);
        T qxz(q.x * q.z);
        T qxy(q.x * q.y);
        T qyz(q.y * q.z);
        T qwx(q.w * q.x);
        T qwy(q.w * q.y);
        T qwz(q.w * q.z);

        matrix[0] = T(1) - T(2) * (qyy +  qzz);
        matrix[4] = T(2) * (qxy + qwz);
        matrix[8] = T(2) * (qxz - qwy);

        matrix[1] = T(2) * (qxy - qwz);
        matrix[5] = T(1) - T(2) * (qxx +  qzz);
        matrix[9] = T(2) * (qyz + qwx);

        matrix[2] = T(2) * (qxz + qwy);
        matrix[6] = T(2) * (qyz - qwx);
        matrix[10] = T(1) - T(2) * (qxx +  qyy);

        matrix = matrix.transpose();
        return matrix;
    }

    mat4 matrixInverse(const mat4& matrix) {
        auto m = matrix.transpose();
        double det;
        robot2D::mat4 inv;
        int i;

        inv[0] = m[5]  * m[10] * m[15] -
                 m[5]  * m[11] * m[14] -
                 m[9]  * m[6]  * m[15] +
                 m[9]  * m[7]  * m[14] +
                 m[13] * m[6]  * m[11] -
                 m[13] * m[7]  * m[10];

        inv[4] = -m[4]  * m[10] * m[15] +
                 m[4]  * m[11] * m[14] +
                 m[8]  * m[6]  * m[15] -
                 m[8]  * m[7]  * m[14] -
                 m[12] * m[6]  * m[11] +
                 m[12] * m[7]  * m[10];

        inv[8] = m[4]  * m[9] * m[15] -
                 m[4]  * m[11] * m[13] -
                 m[8]  * m[5] * m[15] +
                 m[8]  * m[7] * m[13] +
                 m[12] * m[5] * m[11] -
                 m[12] * m[7] * m[9];

        inv[12] = -m[4]  * m[9] * m[14] +
                  m[4]  * m[10] * m[13] +
                  m[8]  * m[5] * m[14] -
                  m[8]  * m[6] * m[13] -
                  m[12] * m[5] * m[10] +
                  m[12] * m[6] * m[9];

        inv[1] = -m[1]  * m[10] * m[15] +
                 m[1]  * m[11] * m[14] +
                 m[9]  * m[2] * m[15] -
                 m[9]  * m[3] * m[14] -
                 m[13] * m[2] * m[11] +
                 m[13] * m[3] * m[10];

        inv[5] = m[0]  * m[10] * m[15] -
                 m[0]  * m[11] * m[14] -
                 m[8]  * m[2] * m[15] +
                 m[8]  * m[3] * m[14] +
                 m[12] * m[2] * m[11] -
                 m[12] * m[3] * m[10];

        inv[9] = -m[0]  * m[9] * m[15] +
                 m[0]  * m[11] * m[13] +
                 m[8]  * m[1] * m[15] -
                 m[8]  * m[3] * m[13] -
                 m[12] * m[1] * m[11] +
                 m[12] * m[3] * m[9];

        inv[13] = m[0]  * m[9] * m[14] -
                  m[0]  * m[10] * m[13] -
                  m[8]  * m[1] * m[14] +
                  m[8]  * m[2] * m[13] +
                  m[12] * m[1] * m[10] -
                  m[12] * m[2] * m[9];

        inv[2] = m[1]  * m[6] * m[15] -
                 m[1]  * m[7] * m[14] -
                 m[5]  * m[2] * m[15] +
                 m[5]  * m[3] * m[14] +
                 m[13] * m[2] * m[7] -
                 m[13] * m[3] * m[6];

        inv[6] = -m[0]  * m[6] * m[15] +
                 m[0]  * m[7] * m[14] +
                 m[4]  * m[2] * m[15] -
                 m[4]  * m[3] * m[14] -
                 m[12] * m[2] * m[7] +
                 m[12] * m[3] * m[6];

        inv[10] = m[0]  * m[5] * m[15] -
                  m[0]  * m[7] * m[13] -
                  m[4]  * m[1] * m[15] +
                  m[4]  * m[3] * m[13] +
                  m[12] * m[1] * m[7] -
                  m[12] * m[3] * m[5];

        inv[14] = -m[0]  * m[5] * m[14] +
                  m[0]  * m[6] * m[13] +
                  m[4]  * m[1] * m[14] -
                  m[4]  * m[2] * m[13] -
                  m[12] * m[1] * m[6] +
                  m[12] * m[2] * m[5];

        inv[3] = -m[1] * m[6] * m[11] +
                 m[1] * m[7] * m[10] +
                 m[5] * m[2] * m[11] -
                 m[5] * m[3] * m[10] -
                 m[9] * m[2] * m[7] +
                 m[9] * m[3] * m[6];

        inv[7] = m[0] * m[6] * m[11] -
                 m[0] * m[7] * m[10] -
                 m[4] * m[2] * m[11] +
                 m[4] * m[3] * m[10] +
                 m[8] * m[2] * m[7] -
                 m[8] * m[3] * m[6];

        inv[11] = -m[0] * m[5] * m[11] +
                  m[0] * m[7] * m[9] +
                  m[4] * m[1] * m[11] -
                  m[4] * m[3] * m[9] -
                  m[8] * m[1] * m[7] +
                  m[8] * m[3] * m[5];

        inv[15] = m[0] * m[5] * m[10] -
                  m[0] * m[6] * m[9] -
                  m[4] * m[1] * m[10] +
                  m[4] * m[2] * m[9] +
                  m[8] * m[1] * m[6] -
                  m[8] * m[2] * m[5];

        det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

        if (det == 0)
            return false;

        det = 1.0 / det;

        robot2D::mat4 invOut;
        for (i = 0; i < 16; i++)
            invOut[i] = inv[i] * det;
        invOut = invOut.transpose();
        return invOut;
    }

}