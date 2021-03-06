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

#include "Drawable.hpp"
#include "Sprite.hpp"
#include "Shader.hpp"
#include "View.hpp"

namespace robot2D {
    namespace priv {
        class RenderImpl;
    }

    class RenderTarget {
    public:
        RenderTarget(const vec2u& size);
        virtual ~RenderTarget() = 0;

        virtual void setView(const View& view);
        virtual const View& getView();
        virtual const View& getDefaultView();
        virtual void draw(const RenderStates& states);
        virtual void draw(const Drawable& drawable, const RenderStates& states
                                                    = RenderStates::Default);

    private:
        void setup();
    protected:
        priv::RenderImpl* m_render;
        ShaderHandler m_spriteShaders;
        unsigned int VAO;
        vec2u m_size;
    };

}
