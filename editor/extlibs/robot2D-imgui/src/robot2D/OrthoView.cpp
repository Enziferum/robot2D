/*********************************************************************
(c) Alex Raag 2024
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

#include "OrthoView.hpp"

namespace robot2D {
    OrthoView::OrthoView():
    m_viewport{},
    needUpdate{false},
    m_matrix{}
    {
    }

    void OrthoView::update(robot2D::FloatRect newViewport) {
        if(m_viewport != newViewport) {
            m_viewport = newViewport;
            needUpdate = true;
        }
    }

    const float* OrthoView::getMatrix() const {
//        mat<4, 4, T, defaultp> Result(1);
//        Result[0][0] = static_cast<T>(2) / (right - left); 0
//        Result[1][1] = static_cast<T>(2) / (top - bottom); 5
//        Result[2][2] = - static_cast<T>(2) / (zFar - zNear); 10
//        Result[3][0] = - (right + left) / (right - left); 12
//        Result[3][1] = - (top + bottom) / (top - bottom); 13
//        Result[3][2] = - (zFar + zNear) / (zFar - zNear); 14

//        mat<4, 4, T, defaultp> Result(static_cast<T>(1));
//        Result[0][0] = static_cast<T>(2) / (right - left);
//        Result[1][1] = static_cast<T>(2) / (top - bottom);
//        Result[2][2] = - static_cast<T>(1);
//        Result[3][0] = - (right + left) / (right - left);
//        Result[3][1] = - (top + bottom) / (top - bottom);

        if(!needUpdate)
            return m_matrix.getRaw();

        float L = m_viewport.lx;
        float R = m_viewport.lx + m_viewport.width;
        float T = m_viewport.lx;
        float B = m_viewport.ly + m_viewport.height;

        m_matrix[0] = 2.0F / (R-L);
        m_matrix[5] = 2.0F / (T-B);
        m_matrix[12] = (R+L) / (L-R);
        m_matrix[13] = (T+B) / (B-T);
        needUpdate = false;
        return m_matrix.getRaw();
    }

} // namespace robot2D