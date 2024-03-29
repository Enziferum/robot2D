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

#include "Rect.hpp"
#include "Transform.hpp"
#include "robot2D/Core/Vector2.hpp"

namespace robot2D {
    /**
     * \brief Provide information about Screen coordinates.
     * \details
     */
    class ROBOT2D_EXPORT_API View {
    public:
        View();
        explicit View(const FloatRect& viewport);
        View(const vec2f& origin, const vec2f& size);
        ~View() = default;

        void setViewport(const FloatRect& viewport);
        const FloatRect& getViewport() const;
        const FloatRect& getRectangle() const;

        void setCenter(const vec2f& origin);
        void setCenter(float x, float y);
        const vec2f& getCenter() const;

        void setSize(const vec2f& size);
        void setSize(float x, float y);
        const vec2f& getSize() const;

        void setRotation(const float& angle);
        const float& getRotation() const;

        void move(const vec2f& offset);
        void move(float x, float y);
        void rotate(const float& angle);
        void zoom(float factor);

        void reset(const FloatRect& rect);

        const Transform& getTransform() const;
        const Transform& getInverseTransform() const;

        void setIsClipping(bool flag) { m_clips = flag; }
        bool isClipping() const { return m_clips; }
    private:
        vec2f m_center;
        vec2f m_size;
        float m_rotation;

        FloatRect m_viewport;
        FloatRect m_rectangle;
        mutable Transform m_transform;
        mutable Transform m_invTransform;
        mutable bool needTransformUpdate;
        mutable bool needInvTransformUpdate;
        bool m_clips{false};
    };
}