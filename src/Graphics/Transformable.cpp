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
#include <cmath>
#include "robot2D/Graphics/Transformable.h"

namespace robot2D{
    Transformable::Transformable()
        :
        m_pos(),
        m_origin(),
        m_scale_factor(),
        m_rotation(0.f),
        m_tranform(),
        m_update_transform(false){

    }

    Transformable::~Transformable() {}

    void Transformable::setPosition(const vec2f& pos) {
        m_pos = pos;
        m_update_transform = true;
    }

    vec2f& Transformable::getPosition() {
        return m_pos;
    }

    void Transformable::setOrigin(const vec2f& origin) {
        m_origin = origin;
        m_update_transform = true;
    }

    vec2f& Transformable::getOrigin() {
        return m_origin;
    }

    void Transformable::setScale(const vec2f& factor) {
        m_scale_factor = factor;
        m_update_transform = true;
    }

    vec2f& Transformable::getScale() {
        return m_scale_factor;
    }

    void Transformable::setRotate(const float &angle) {

        m_rotation = static_cast<float>(fmod(angle, 360));
        if(m_rotation < 0 )
            m_rotation += 360.f;
        m_update_transform = true;
    }

    float& Transformable::getRotate() {
        return m_rotation;
    }

    void Transformable::move(const vec2f &offset) {
        setPosition(vec2f(m_pos.x + offset.x, m_pos.y + offset.y));
    }

    void Transformable::scale(const vec2f &factor) {
        setScale(vec2f(m_scale_factor.x * factor.x,
                       m_scale_factor.y * factor.y));
    }

    void Transformable::rotate(float angle) {
        setRotate(m_rotation + angle);
    }

    const Transform& Transformable::getTransform() const {
        if(m_update_transform){
            //convert to radians
            float angle  = -m_rotation * 3.141592654f / 180.f;
            float cosine = static_cast<float>(std::cos(angle));
            float sine   = static_cast<float>(std::sin(angle));
            float sxc    = m_scale_factor.x * cosine;
            float syc    = m_scale_factor.y * cosine;
            float sxs    = m_scale_factor.x * sine;
            float sys    = m_scale_factor.y * sine;
            float tx     = -m_origin.x * sxc - m_origin.y * sys + m_pos.x;
            float ty     =  m_origin.x * sxs - m_origin.y * syc + m_pos.y;

            m_tranform = Transform( sxc, sys, tx,
                                     -sxs, syc, ty,
                                     0.f, 0.f, 1.f);
            m_update_transform = false;
        }
        return m_tranform;
    }
}