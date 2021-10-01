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

#include <robot2D/Graphics/RenderStates.hpp>
#include <robot2D/Graphics/View.hpp>
#include <robot2D/Graphics/Vertex.hpp>
#include <robot2D/Graphics/RenderStats.hpp>

namespace robot2D {
    namespace priv {
        class RenderImpl {
        public:
            RenderImpl();
            virtual ~RenderImpl() = 0;

            static RenderImpl* create();
            virtual void render(const VertexData& data, const RenderStates& states) const = 0;

            virtual void setView(const View& view) = 0;
            virtual const View& getView() = 0;
            virtual const View& getDefaultView() = 0;
            virtual void setSize(const vec2u& size);

            virtual void beforeRender() const = 0;
            virtual void afterRender() const = 0;
            virtual void flushRender() const = 0;
            virtual const RenderStats& getStats() const = 0;
        protected:
            vec2u m_size;
        };
    }
}
