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
#include <cstdint>
#include <robot2D/Core/Vector3.hpp>

namespace robot2D {
    class Matrix3D {
    public:
        // Default Matrix ( Identity )
        Matrix3D();
        Matrix3D(float a0, float a1, float a2, float a3,
                 float a4, float a5, float a6, float a7,
                 float a8, float a9, float a10, float a11,
                 float a12, float a13, float a14, float a15);

        Matrix3D(const float& value);
        Matrix3D(const Matrix3D& other) {
            for(int it = 0; it < 16; ++it)
                m_matrix[it] = other.m_matrix[it];
        }
        Matrix3D& operator=(const Matrix3D& other) {
            if (other == *this)
                return *this;
            for(int it = 0; it < 16; ++it)
                m_matrix[it] = other.m_matrix[it];
            return *this;
        }

        ~Matrix3D() = default;

        const float& operator[](std::size_t index) const {
            return m_matrix[index];
        }

        float& operator[](std::size_t index) {
            return m_matrix[index];
        }

        float* getRaw() { return m_matrix; }
        const float* getRaw() const { return &m_matrix[0]; }

        static Matrix3D Identity;
        Matrix3D operator*(const Matrix3D& right);

        float& operator()(std::size_t row, std::size_t column) {
            return m_matrix[row * 4 + column];
        }

        const float& operator()(std::size_t row, std::size_t column) const {
            return m_matrix[row * 4 + column];
        }

        friend bool operator != (const Matrix3D& left, const Matrix3D& right);
        friend bool operator == (const Matrix3D& left, const Matrix3D& right);

        [[nodiscard]] Matrix3D transpose() const;

        vec3f transformVector(const vec3f& vector) const;
    private:
        float m_matrix[16];
    };

    using mat4 = Matrix3D;

    vec3f operator* (const Matrix3D& left, const vec3f& vec);

    /// Matrix Operations ///

    mat4 translate(mat4 const& matrix, robot2D::vec3f const& to);

    mat4 scale(mat4 const& matrix, const robot2D::vec3f& scaleFactor);

    /// Matrix Operations ///
}