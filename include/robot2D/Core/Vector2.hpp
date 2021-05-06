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

namespace robot2D{
    template<typename T>
    struct ROBOT2D_EXPORT_API Vector2{
        Vector2();
        Vector2(const T& x, const T& y);

        Vector2(const Vector2& other);
        Vector2& operator=(const Vector2& other);
        ~Vector2() = default;

        Vector2& operator+= (const Vector2& other);
        Vector2& operator-= (const Vector2& other);
        Vector2 operator* (const T& value);

        friend Vector2 operator+ (Vector2 l, const Vector2& r){
            l += r;
            return l;
        }

        friend Vector2 operator- (Vector2 l, const Vector2& r){
            l -= r;
            return l;
        }

        T x;
        T y;
    };

    using vec2f = Vector2<float>;
    using vec2i = Vector2<int>;
    using vec2u = Vector2<unsigned int>;

    template<typename T>
    Vector2<T>::Vector2(): x(0), y(0) {

    }

    template<typename T>
    Vector2<T>::Vector2(const T& _x, const T& _y):x(_x), y(_y)  {

    }

    template<typename T>
    Vector2<T>::Vector2(const Vector2& other):x(other.x), y(other.y) {

    }

    template<typename T>
    Vector2<T> &Vector2<T>::operator=(const Vector2& other) {
        if( *this == other)
            return *this;

        x = other.x;
        y = other.y;

        return *this;
    }

    template<typename T>
    Vector2<T> &Vector2<T>::operator+=(const Vector2 &other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    template<typename T>
    Vector2<T> &Vector2<T>::operator-=(const Vector2 &other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    template<typename T>
    Vector2<T> Vector2<T>::operator*(const T& value) {
        x *= value;
        y *= value;
        return *this;
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