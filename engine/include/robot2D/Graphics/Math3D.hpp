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

#pragma once

#include "Matrix3D.hpp"

namespace robot2D {

    struct Quaternion {
        Quaternion();
        Quaternion(float x, float y, float z, float w);
        Quaternion(robot2D::vec3f eulerAngle);
        ~Quaternion() = default;

        Quaternion Conjugate() const;
        friend robot2D::vec3f operator *(const Quaternion& quat, const vec3f& vec);

        float x;
        float y;
        float z;
        float w;
    };

    using quat = Quaternion;

    mat4 matrixInverse(const robot2D::mat4& matrix);

    mat4 toMat4(const quat& quat);

    /// Projection Perspective Camera
    mat4 projectionPerspective(float fov, float aspectRatio, float zNear, float zFar);

    /// Cross 2 3D-Vectors
    vec3f cross(const vec3f& left, const vec3f& right);

    /// Rotate Using Quatersion
    vec3f rotate(quat const& quat, const vec3f& axis);

    /// apply cos function to all 3 axis
    vec3f cosVec3F(const vec3f& in);

    /// apply sin function to all 3 axis
    vec3f sinVec3F(const vec3f& in);
}