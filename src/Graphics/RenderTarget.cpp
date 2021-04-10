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

#include <ext/glad.h>

#include "RenderImpl.hpp"
#include "robot2D/Graphics/RenderTarget.h"

namespace robot2D {

    void RenderTarget::ortho_projection(Matrix& m, float l, float r, float b,
                  float t, float n, float f){
        m.mat[0][0] = 2 / (r - l);

        m.mat[0][1] = 0;
        m.mat[0][2] = 0;
        m.mat[0][3] = 0;

        m.mat[1][0] = 0;
        m.mat[1][1] = 2 / (t - b);
        m.mat[1][2] = 0;
        m.mat[1][3] = 0;

        m.mat[2][0] = 0;
        m.mat[2][1] = 0;
        m.mat[2][2] = -2 / (f - n);
        m.mat[2][3] = 0;

        m.mat[3][0] = -(r + l) / (r - l);
        m.mat[3][1] = -(t + b) / (t - b);
        m.mat[3][2] = -(f + n) / (f - n);
        m.mat[3][3] = 1;
    }


    RenderTarget::RenderTarget(const vec2u& size):
        m_size(size),
        m_renderImpl(nullptr) {
    }

    RenderTarget::~RenderTarget() {
        if(m_renderImpl != nullptr){
            delete m_renderImpl;
            m_renderImpl = nullptr;
        }
    }


    void RenderTarget::draw(const Drawable& drawable, const RenderStates& states) {
        drawable.draw(*this, states);
    }

    // todo work on cache
    void RenderTarget::draw(const RenderStates& states) {
        m_renderImpl -> preprocess();
        m_renderImpl -> process();
        m_renderImpl -> postprocess();
    }

    const Matrix& RenderTarget::projection_matrix() const {
        return m_renderImpl -> projection_matrix();
    }

    void RenderTarget::create() {
        m_renderImpl = priv::Render::create();
    }

}