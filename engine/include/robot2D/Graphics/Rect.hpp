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

#include <algorithm>
#include <robot2D/Core/Vector2.hpp>

namespace robot2D {
    template<typename T>
    struct Rect {
        Rect();
        Rect(const T& leftx, const T& lefty, const T& width, const T& height);
        Rect& operator=(const Rect& other);
        Rect(const Rect& other);
        ~Rect() = default;

        bool contains(const Vector2<T>& point);

        T lx;
        T ly;
        T width;
        T height;
    };

    template<typename T>
    Rect<T>::Rect():
    lx(0),
    ly(0),
    width(0),
    height(0){

    }

    template<typename T>
    Rect<T>::Rect(const T& leftx, const T &lefty, const T& w, const T& h)
    : lx(leftx), ly(lefty), width(w), height(h) {
    }

    template<typename T>
    Rect<T>::Rect(const Rect& other):
    lx(other.lx), ly(other.ly), width(other.width), height(other.height) {

    }

    template<typename T>
    bool Rect<T>::contains(const Vector2<T>& point) {
        T minX = std::min(lx, static_cast<T>(lx + width));
        T maxX = std::max(lx, static_cast<T>(lx + width));
        T minY = std::min(ly, static_cast<T>(ly + height));
        T maxY = std::max(ly, static_cast<T>(ly + height));

        return (point.x >= minX) && (point.x < maxX) && (point.y >= minY)
        && (point.y < maxY);
    }

    template<typename T>
    Rect<T> &Rect<T>::operator=(const Rect &other) {
        if(*this == other)
            return *this;

        lx = other.lx;
        ly = other.ly;
        width = other.width;
        height = other.height;

        return *this;
    }

    template<typename T>
    bool operator ==(const Rect<T>& lhs, const Rect<T>& rhs){
        return (lhs.lx == rhs.lx ) && (lhs.ly == rhs.ly) && (lhs.width == rhs.width)
        && (lhs.height == rhs.height);
    }

    template<typename T>
    bool operator !=(const Rect<T>& lhs, const Rect<T>& rhs){
        return !(lhs==rhs);
    }

    using FloatRect = Rect<float>;
    using IntRect = Rect<int>;
}