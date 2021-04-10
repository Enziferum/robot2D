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

#include <robot2D/Core/Vector2.h>
#include "Rect.h"

namespace robot2D {

    /**
     * \brief 2D Transform matrix, allows to get different transformations
     */
    class Transform{
    public:
        Transform();
        Transform(float, float, float,
                  float, float, float,
                  float, float, float);

        ~Transform() = default;

        vec2f transformPoint(float x, float y) const;
        vec2f transformPoint(const vec2f& ) const;

        FloatRect transformRect(const FloatRect& rectangle) const;

        const float* get_matrix()const;
        Transform& combine(const Transform& other);

        Transform& translate(float x, float y);
        Transform& translate(const vec2f& offset);

        Transform& rotate(float angle);
        Transform& rotate_around(float angle, vec2f point);

        Transform& scale(float x, float y);
        Transform& scale(const vec2f& factor);
    private:
        float m_mat[16];
    };

    Transform operator *(const Transform& left, const Transform& right);
    Transform& operator *=(Transform& left, const Transform& right);
    vec2f operator *(const Transform& left, const vec2f& right);
    bool operator ==(const Transform& left, const Transform& right);
    bool operator !=(const Transform& left, const Transform& right);
}