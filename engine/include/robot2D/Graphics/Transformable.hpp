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

#include "Transform.hpp"
#include "robot2D/Core/Vector2.hpp"

namespace robot2D {
    /**
     * \brief This class allows to be your custom object
     */
    class Transformable {
    public:
        Transformable();
        virtual ~Transformable() = 0;


        virtual void setPosition(const vec2f& pos);
        vec2f& getPosition();

        virtual void setOrigin(const vec2f& origin);
        vec2f& getOrigin();

        virtual void setSize(const vec2f& size);
        virtual void setSize(const float& x, const float& y);
        vec2f& getSize();

        virtual void setScale(const vec2f& factor);
        vec2f& getScale();

        virtual void setRotate(const float& angle);
        float& getRotate();

        void move(const vec2f& offset);
        void scale(const vec2f& factor);
        void rotate(float angle);


        const Transform& getTransform() const;

    protected:
         vec2f m_pos;
         vec2f m_origin;
         vec2f m_scale_factor;
         vec2f m_size;

         float m_rotation;
         mutable Transform m_transform;
         mutable bool m_update_transform;
    };
}