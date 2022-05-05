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

#include <robot2D/Config.hpp>

namespace robot2D {

    /**
     * \brief 2-dimensional format of storing Data. Usefull for Coordinate system.
     * @tparam T internal x, y format. Using prebased formats for float, int, unsigned int
     */
    template<typename T>
    struct ROBOT2D_EXPORT_API Vector2 {
        constexpr Vector2();
        constexpr Vector2(const T& x, const T& y);

        template<typename U>
        constexpr explicit Vector2(const Vector2<U>& other);

       // Vector2& operator=(const Vector2& other);
        ~Vector2() = default;

        template<typename U>
        constexpr Vector2<U> as() {
            return Vector2<U> {
                static_cast<U>(x),
                static_cast<U>(y)
            };
        }

        T x;
        T y;
    };

    using vec2f = Vector2<float>;
    using vec2i = Vector2<int>;
    using vec2u = Vector2<unsigned int>;

    template<typename T>
    constexpr Vector2<T>::Vector2():
    x(0), y(0) {}

    template<typename T>
    constexpr Vector2<T>::Vector2(const T& _x, const T& _y):
    x(_x), y(_y)  {}

    template<typename T>
    template<typename U>
    constexpr Vector2<T>::Vector2(const Vector2<U>& other):
            x(static_cast<T>(other.x)),
            y(static_cast<T>(other.y)){}

    template<typename T>
    constexpr Vector2<T> operator+ (Vector2<T> left, const Vector2<T>& right){
        return Vector2<T>(left.x + right.x, left.y + right.y);
    }

    template <typename T>
    constexpr Vector2<T> operator -(const Vector2<T>& left, const Vector2<T>& right)
    {
        return Vector2<T>(left.x - right.x, left.y - right.y);
    }


    template<typename T>
    constexpr Vector2<T> operator- (const Vector2<T>& right){
       return Vector2<T>(-right.x, -right.y);
    }


    template<typename T>
    constexpr Vector2<T>& operator +=(Vector2<T>& left, const Vector2<T>& right) {
        left.x += right.x;
        left.y += right.y;
        return left;
    }

    template<typename T>
    constexpr Vector2<T>& operator -=(Vector2<T>& left, const Vector2<T>& right) {
        left.x -= right.x;
        left.x -= right.y;
        return left;
    }

    template<typename T>
    constexpr Vector2<T> operator*(const Vector2<T>& left, T right) {
       return Vector2<T>(left.x * right, left.y * right);
    }

    template <typename T>
    constexpr Vector2<T>& operator *=(Vector2<T>& left, T right)
    {
        left.x *= right;
        left.y *= right;

        return left;
    }

    template <typename T>
    constexpr Vector2<T> operator /(const Vector2<T>& left, T right)
    {
        return Vector2<T>(left.x / right, left.y / right);
    }

    template <typename T>
    constexpr Vector2<T>& operator /=(Vector2<T>& left, T right)
    {
        left.x /= right;
        left.y /= right;

        return left;
    }

    template<typename T>
    bool operator == (const Vector2<T>& lhs, const Vector2<T>& rhs){
        return (lhs.x == rhs.x ) && (lhs.y == rhs.y);
    }

    template<typename T>
    bool operator != (const Vector2<T>& lhs, const Vector2<T>& rhs){
        return !(lhs == rhs );
    }
}