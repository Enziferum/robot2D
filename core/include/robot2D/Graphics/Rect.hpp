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
#include <array>

#include <robot2D/Config.hpp>
#include <robot2D/Core/Vector2.hpp>

namespace robot2D {
    /**
     * \brief Rectangle in 2D Space.
     * \details
     * @tparam T
     */
    template<typename T>
    struct ROBOT2D_EXPORT_API Rect {
        /// All values are zero
        Rect();

        /// \brief Specifies TopLeft Coordinate with width and height.
        /// \details These information allows to calculate all 4 coordinates.
        Rect(const T& leftx, const T& lefty, const T& width, const T& height);
        Rect(const Vector2<T>& topLeft, const Vector2<T>& size);


        Rect(const Rect& other);
        Rect& operator=(const Rect& other);

        template<typename U>
        Rect(const Rect<U>& other);

        template<typename U>
        Rect<T>& operator=(const Rect<U>& other);

        ~Rect() = default;


        /// \brief Check if current rect intersects with other rect.
        bool intersects(const Rect<T>& rect) const;

        /// \brief Check if current rect intersects with other rect.
        /// \details These methods not only checks intersection.
        /// Also it providers intersection rectangle in other words overlapping \n
        /// Between 2 rectangles.
        bool intersects(const Rect<T>& other, Rect<T>& overlap) const;

        /// Check if point in rect bounds
        bool contains(const Vector2<T>& point) const;

        /// Check if current rect whole contains other rect
        bool contains(const Rect<T>& other) const;

        template<typename U>
        Rect<U> as() const;

        /// Create Rect from 2 Points, topLeft{lx, ly} and bottomRight {lx + width, ly + height}
        static Rect<T> create(const Vector2<T>& topPoint, const Vector2<T>& botPoint);
        /// Create Rect from 4 Points, using them like anchors
        static Rect<T> create(const Vector2<T>& point1,
                              const Vector2<T>& point2,
                              const Vector2<T>& point3,
                              const Vector2<T>& point4);

        Vector2<T> centerPoint() const {
            return { lx + width / 2, ly + height / 2 };
        }

        /// {lx, ly}
        Vector2<T> topPoint() const {
            return {lx, ly};
        }

        /// {lx + width, ly}
        Vector2<T> widthPoint() const {
            return {lx + width, ly};
        }

        /// {lx, ly + height}
        Vector2<T> heightPoint() const {
            return {lx, ly + height};
        }

        /// {lx + witdh, ly + height}
        Vector2<T> botPoint() const {
            return {lx + width, ly + height};
        }

        T area() const {
            if(m_area == 0)
                m_area = width * height;
            return m_area;
        }

        bool isRotated() const {
            return m_rotateAngle != 0.f;
        }

        void setRotateAngle(float angle) {
            m_rotateAngle = angle;
        }

        const float& getRotateAngle() const {
            return m_rotateAngle;
        }

        T lx;
        T ly;
        T width;
        T height;
    private:
        mutable T m_area{0};
        float m_rotateAngle { 0.f };
    };

    template<typename T>
    Rect<T>::Rect():
    lx(0),
    ly(0),
    width(0),
    height(0){}

    template<typename T>
    Rect<T>::Rect(const T& leftx, const T &lefty, const T& w, const T& h)
    : lx(leftx), ly(lefty), width(w), height(h) {}

    template<typename T>
    Rect<T>::Rect(const Rect& other):
    lx(other.lx), ly(other.ly), width(other.width), height(other.height), m_area{other.m_area}, m_rotateAngle{other.m_rotateAngle} {}

    template<typename T>
    template<typename U>
    Rect<T>::Rect(const Rect<U>& other): lx{other.lx}, ly{other.ly}, width{other.width},
    height{other.height}, m_area{other.m_area}, m_rotateAngle{other.m_rotateAngle} {}

    template<typename T>
    bool Rect<T>::contains(const Vector2<T>& point) const  {
        T minX = std::min(lx, static_cast<T>(lx + width));
        T maxX = std::max(lx, static_cast<T>(lx + width));
        T minY = std::min(ly, static_cast<T>(ly + height));
        T maxY = std::max(ly, static_cast<T>(ly + height));

        return (point.x >= minX) && (point.x < maxX) && (point.y >= minY)
        && (point.y < maxY);
    }

    template<typename T>
    bool Rect<T>::intersects(const Rect<T>& other) const {
        Rect<T> overlap;
        return intersects(other, overlap);
    }


    template<typename T>
    bool Rect<T>::intersects(const Rect<T>& other, Rect<T>& overlap) const {
        T minX = std::min(lx, static_cast<T>(lx + width));
        T maxX = std::max(lx, static_cast<T>(lx + width));
        T minY = std::min(ly, static_cast<T>(ly + height));
        T maxY = std::max(ly, static_cast<T>(ly + height));

        T minX2 = std::min(other.lx, static_cast<T>(other.lx + other.width));
        T maxX2 = std::max(other.lx, static_cast<T>(other.lx + other.width));
        T minY2 = std::min(other.ly, static_cast<T>(other.ly + other.height));
        T maxY2 = std::max(other.ly, static_cast<T>(other.ly + other.height));

        T innerLeft = std::max(minX, minX2);
        T innerTop = std::max(minY, minY2);
        T innerRight = std::min(maxX, maxX2);
        T innerBottom = std::min(maxY, maxY2);

        bool result = false;
        if(((innerLeft <= innerRight) && (innerTop <= innerBottom))) {
            overlap = {innerLeft, innerTop, innerRight - innerLeft, innerBottom - innerTop};
            result = true;
        } else {
            overlap = {};
            result = false;
        }

        return result;
    }


    template<typename T>
    Rect<T>& Rect<T>::operator=(const Rect& other) {
        if(*this == other)
            return *this;

        lx = other.lx;
        ly = other.ly;
        width = other.width;
        height = other.height;
        m_area = other.m_area;
        m_rotateAngle = other.m_rotateAngle;

        return *this;
    }

    template<typename T>
    template<typename U>
    Rect<T>& Rect<T>::operator=(const Rect<U>& other) {
        lx = static_cast<T>(other.lx);
        ly = static_cast<T>(other.ly);
        width = static_cast<T>(other.width);
        height = static_cast<T>(other.height);
        m_area = static_cast<T>(other.m_area);
        m_rotateAngle = other.m_rotateAngle;
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

    template<typename T>
    Rect<T> Rect<T>::create(const Vector2<T>& topPoint, const Vector2<T>& botPoint) {
        return {topPoint.x, topPoint.y, botPoint.x - topPoint.x, botPoint.y - topPoint.y};
    }

    template<typename T>
    Rect<T> Rect<T>::create(const Vector2<T>& point1, const Vector2<T>& point2,
                            const Vector2<T>& point3, const Vector2<T>& point4)  {
        Vector2<T> topPoint = point1;
        Vector2<T> botPoint = point1;

        std::array<robot2D::Vector2<T>, 4> points = {point1, point2, point3, point4};

        for(int i = 0; i < 3; ++i) {
            if(points[i].x > points[i + 1].x)
                topPoint.x = points[i + 1].x;
            if(points[i].y > points[i + 1].y)
                topPoint.y = points[i + 1].y;
        }

        for(int i = 0; i < 3; ++i) {
            if(points[i].x < points[i + 1].x)
                botPoint.x = points[i + 1].x;
            if(points[i].y < points[i + 1].y)
                botPoint.y = points[i + 1].y;
        }

        return create(topPoint, botPoint);
    }

    template<typename T>
    bool Rect<T>::contains(const Rect<T>& other) const {
        if (lx <= other.lx && ly <= other.ly
            && (lx + width) >= (other.lx + other.width)
            && (ly + height) >= (other.ly + other.height))
            return true;
        else
            return false;
    }

    template<typename T>
    template<typename U>
    Rect<U> Rect<T>::as() const {
        return {
                static_cast<U>(lx),
                static_cast<U>(ly),
                static_cast<U>(width),
                static_cast<U>(height)
        };
    }

    template<typename T>
    Rect<T>::Rect(const Vector2<T>& topLeft, const Vector2<T>& size):
    lx{topLeft.x}, ly{topLeft.y}, width{size.x}, height{size.y} {}

    template<typename T>
    bool operator<(const Rect<T>& left, const Rect<T>& right) {
        return (left.lx < right.lx) && (left.ly < right.ly)
            && (left.width < right.width) && (left.height < right.height);
    }


    using FloatRect = Rect<float>;
    using IntRect = Rect<int>;
    using UIntRect = Rect<unsigned int>;
}