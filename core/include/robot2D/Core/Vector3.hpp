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

#include <robot2D/Config.hpp>
#include <type_traits>
#include "Vector2.hpp"

namespace robot2D {

    /** @brief A 3-component value with componentwise arithmetic.
     * Arithmetic scalar types support constexpr and noexcept operations.
     * Other scalar types remain supported with conservative exception specifications;
     * copy/move exception guarantees follow the scalar type. Conversions use
     * static_cast, preserving its range/truncation rules. Integer overflow and
     * division by zero are not checked. Instances have no shared state.
     * @tparam T Component type; predefined aliases use float, int and unsigned int.
     */
    template<typename T>
    struct ROBOT2D_EXPORT_API Vector3 {
        constexpr Vector3() noexcept(std::is_arithmetic_v<T>);
        constexpr Vector3(const T& x, const T& y, const T& z) noexcept(std::is_arithmetic_v<T>);
        constexpr Vector3(const Vector2<T>& vec2) noexcept(std::is_arithmetic_v<T>);

        template<typename U>
        constexpr Vector3(const Vector2<U>& vec2) noexcept(std::is_arithmetic_v<T> && std::is_arithmetic_v<U>);

        template<typename U>
        constexpr explicit Vector3(const Vector3<U>& other) noexcept(std::is_arithmetic_v<T> &&
        std::is_arithmetic_v<U>);

        constexpr Vector3(const Vector3& other) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
        constexpr Vector3& operator=(const Vector3& other) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
        constexpr Vector3(Vector3&& other) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
        constexpr Vector3& operator=(Vector3&& other) noexcept(std::is_nothrow_move_assignable_v<T>) = default;
        ~Vector3() = default;

        /// Returns a componentwise scalar conversion; the source is unchanged.
        template<typename U>
        constexpr Vector3<U> as() const noexcept(std::is_arithmetic_v<T> && std::is_arithmetic_v<U>) {
            return Vector3<U> {
                static_cast<U>(x),
                static_cast<U>(y),
                static_cast<U>(z),
            };
        }

        /// Converts x/y and discards z; the source is unchanged.
        template<typename U>
        constexpr Vector2<U> asVec2() const noexcept(std::is_arithmetic_v<T> && std::is_arithmetic_v<U>) {
            return Vector2<U> {
                static_cast<U>(x),
                static_cast<U>(y)
            };
        }

        T x;
        T y;
        T z;
    };

    using vec3f = Vector3<float>;
    using vec3i = Vector3<int>;
    using vec3u = Vector3<unsigned int>;

    template<typename T>
    constexpr Vector3<T>::Vector3() noexcept(std::is_arithmetic_v<T>) :
        x(0), y(0), z{0} {
    }

    template<typename T>
    constexpr Vector3<T>::Vector3(const T& _x, const T& _y, const T& _z) noexcept(std::is_arithmetic_v<T>) :
        x(_x), y(_y), z{_z} {
    }

    template<typename T>
    constexpr Vector3<T>::Vector3(const Vector2<T>& other) noexcept(std::is_arithmetic_v<T>) :
        x(other.x), y(other.y), z(0) {
    }


    template<typename T>
    template<typename U>
    constexpr Vector3<T>::Vector3(const Vector2<U>& other) noexcept(std::is_arithmetic_v<T> &&
    std::is_arithmetic_v<U>) :
        x(static_cast<T>(other.x)),
        y(static_cast<T>(other.y)),
        z(0) {
    }

    template<typename T>
    template<typename U>
    constexpr Vector3<T>::Vector3(const Vector3<U>& other) noexcept(std::is_arithmetic_v<T> &&
    std::is_arithmetic_v<U>) :
        x(static_cast<T>(other.x)),
        y(static_cast<T>(other.y)),
        z(static_cast<T>(other.z)) {
    }

    template<typename T>
    constexpr Vector3<T> operator+(Vector3<T> left, const Vector3<T>& right) noexcept(std::is_arithmetic_v<T>) {
        return Vector3<T>(left.x + right.x, left.y + right.y, left.z + right.z);
    }

    template <typename T>
    constexpr Vector3<T> operator -(const Vector3<T>& left, const Vector3<T>& right) noexcept(std::is_arithmetic_v<T>) {
        return Vector3<T>(left.x - right.x, left.y - right.y, left.z - right.z);
    }


    template<typename T>
    constexpr Vector3<T> operator-(const Vector3<T>& right) noexcept(std::is_arithmetic_v<T>) {
        return Vector3<T>(-right.x, -right.y, -right.z);
    }


    template<typename T>
    constexpr Vector3<T>& operator +=(Vector3<T>& left, const Vector3<T>& right) noexcept(std::is_arithmetic_v<T>) {
        left.x += right.x;
        left.y += right.y;
        left.z += right.z;
        return left;
    }

    template<typename T>
    constexpr Vector3<T>& operator -=(Vector3<T>& left, const Vector3<T>& right) noexcept(std::is_arithmetic_v<T>) {
        left.x -= right.x;
        left.y -= right.y;
        left.z -= right.z;
        return left;
    }

    template<typename T>
    constexpr Vector3<T> operator*(const Vector3<T>& left, T right) noexcept(std::is_arithmetic_v<T>) {
        return Vector3<T>(left.x * right, left.y * right, left.z * right);
    }

    template <typename T>
    constexpr Vector3<T>& operator *=(Vector3<T>& left, T right) noexcept(std::is_arithmetic_v<T>) {
        left.x *= right;
        left.y *= right;
        left.z *= right;

        return left;
    }

    template <typename T>
    constexpr Vector3<T> operator /(const Vector3<T>& left, T right) noexcept(std::is_arithmetic_v<T>) {
        return Vector3<T>(left.x / right, left.y / right, left.z / right);
    }

    template <typename T>
    constexpr Vector3<T>& operator /=(Vector3<T>& left, T right) noexcept(std::is_arithmetic_v<T>) {
        left.x /= right;
        left.y /= right;
        left.z /= right;

        return left;
    }

    template<typename T>
    constexpr bool operator ==(const Vector3<T>& lhs, const Vector3<T>& rhs) noexcept(std::is_arithmetic_v<T>) {
        return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
    }

    template<typename T>
    constexpr bool operator !=(const Vector3<T>& lhs, const Vector3<T>& rhs) noexcept(std::is_arithmetic_v<T>) {
        return !(lhs == rhs);
    }
}