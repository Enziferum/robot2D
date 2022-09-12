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
}