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
#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Graphics/View.hpp>
#include <robot2D/Graphics/RenderStats.hpp>

#include "../RenderImpl.hpp"
#include "RenderBuffer.hpp"

namespace robot2D {
    using uint = unsigned int;

    namespace priv {
        class OpenGLRender: public RenderImpl {
        public:
            OpenGLRender();
            ~OpenGLRender();

            void render(const RenderStates& states) override;
            void render(const VertexData& data, const RenderStates& states) const override;

            void setSize(const vec2u& size) override;
            void setView(const View& view) override;

            const View& getView() override;
            const View& getDefaultView() override;
            virtual const RenderStats& getStats() const override;
        private:
            void init();
            void destroy();

            virtual void beforeRender() const override;
            virtual void afterRender()const override;
            virtual void flushRender() const override;

            IntRect getViewport(const View& view);

            void applyCurrentView();
        private:
            mutable RenderBuffer m_renderBuffer;
            ShaderHandler m_quadShader;
            View m_view;
            View m_default;
            mutable RenderStats m_stats;
        };
    }
}
