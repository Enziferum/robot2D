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

#include "Drawable.h"
#include "Sprite.h"
#include "Shader.h"


namespace robot2D {
    struct matrix {
        float mat[4][4];
    };

    class RenderTarget{
    public:
        RenderTarget(const vec2u& size);
        virtual ~RenderTarget() = 0;

        virtual void draw(const RenderStates& states);
        virtual void draw(const Drawable& drawable, const RenderStates& states
                                                    = RenderStates::Default);


    protected:
        void ortho_projection(matrix& m, float l, float r, float b,
                              float t, float n, float f);
    private:
        virtual void setup_GL();

    protected:
        ShaderHandler m_spriteShaders;
        unsigned int VAO;

        vec2u m_size;
        matrix mat;
    };
}
