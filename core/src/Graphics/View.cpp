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
#include <robot2D/Graphics/View.hpp>

namespace robot2D {

    View::View():
            m_center             (),
            m_size               (),
            m_rotation           (0),
            m_viewport           (0, 0, 1, 1),
            needTransformUpdate   (false),
            needInvTransformUpdate{false} {
        reset(FloatRect(0, 0 , 1000, 1000));
    }

    View::View(const FloatRect& viewport):
            m_center             (),
            m_size               (),
            m_rotation           (0),
            m_viewport           (0, 0, 1, 1),
            needTransformUpdate   (false),
            needInvTransformUpdate{false} {
        reset(viewport);
    }

    View::View(const vec2f& center, const vec2f& size):
            m_center             (center),
            m_size               (size),
            m_rotation           (0),
            m_viewport           (0, 0, 1, 1),
            needTransformUpdate   (false),
            needInvTransformUpdate{false} {

    }

    void View::setViewport(const FloatRect& viewport) {
        m_viewport = viewport;
        needTransformUpdate = true;
    }

    const FloatRect& View::getViewport() const {
        return m_viewport;
    }

    void View::setCenter(const vec2f& center) {
        m_center = center;
        m_rectangle.lx = center.x - m_rectangle.width / 2.f;
        m_rectangle.ly = center.y - m_rectangle.height / 2.f;
        needTransformUpdate = true;
        needInvTransformUpdate = true;
    }

    void View::setCenter(float x, float y) {
        m_center.x = x;
        m_center.y = y;
        m_rectangle.lx = x - m_rectangle.width / 2.f;
        m_rectangle.ly = y - m_rectangle.height / 2.f;
        needTransformUpdate = true;
        needInvTransformUpdate = true;
    }

    const vec2f& View::getCenter() const {
        return m_center;
    }

    void View::setSize(const vec2f& size) {
        m_size = size;
        needTransformUpdate = true;
        needInvTransformUpdate = true;
    }

    void View::setSize(float x, float y) {
        m_size.x = x;
        m_size.y = y;
        m_rectangle.width = m_size.x;
        m_rectangle.height = m_size.y;
        needTransformUpdate = true;
        needInvTransformUpdate = true;
    }

    const vec2f& View::getSize() const {
        return m_size;
    }

    void View::setRotation(const float &angle){
        m_rotation = static_cast<float>(fmod(angle, 360));
        if (m_rotation < 0)
            m_rotation += 360.f;
        needTransformUpdate = true;
        needInvTransformUpdate = true;
    }

    const float& View::getRotation() const {
        return m_rotation;
    }

    void View::move(const vec2f& offset) {
        setCenter(m_center + offset);
    }

    void View::move(float x, float y) {
        setCenter(m_center.x + x, m_center.y + y);
    }

    void View::rotate(const float &angle) {
        setRotation(m_rotation + angle);
    }

    void View::zoom(float factor) {
        setSize(m_size.x * factor, m_size.y * factor);
    }

    void View::reset(const FloatRect& rectangle) {
        m_center.x = rectangle.lx + rectangle.width / 2.f;
        m_center.y = rectangle.ly + rectangle.height / 2.f;
        m_size.x   = rectangle.width;
        m_size.y   = rectangle.height;
        m_rotation = 0;
        m_rectangle = rectangle;
        needTransformUpdate = true;
        needInvTransformUpdate = true;
    }

    const Transform& View::getTransform() const {
        if(needTransformUpdate) {

            float angle  = m_rotation * 3.141592654f / 180.f;
            float cosine = static_cast<float>(std::cos(angle));
            float sine   = static_cast<float>(std::sin(angle));
            float tx     = -m_center.x * cosine - m_center.y * sine + m_center.x;
            float ty     =  m_center.x * sine - m_center.y * cosine + m_center.y;

            // Projection components
            float a =  2.f / m_size.x;
            float b = -2.f / m_size.y;
            float c = -a * m_center.x;
            float d = -b * m_center.y;

            // Rebuild the projection matrix
            m_transform = Transform( a * cosine, a * sine,   a * tx + c,
                                     -b * sine,   b * cosine, b * ty + d,
                                     0.f,        0.f,        1.f);
            needTransformUpdate = false;
        }
        return m_transform;
    }

    const Transform& View::getInverseTransform() const {
        if (needInvTransformUpdate)
        {
            m_invTransform = getTransform().getInverse();
            needInvTransformUpdate = false;
        }

        return m_invTransform;
    }

    const FloatRect &View::getRectangle() const {
        return m_rectangle;
    }
}