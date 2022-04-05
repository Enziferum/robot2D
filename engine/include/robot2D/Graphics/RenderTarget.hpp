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
#include <memory>

#include <robot2D/Config.hpp>
#include "Drawable.hpp"
#include "Sprite.hpp"
#include "Shader.hpp"
#include "View.hpp"
#include "Vertex.hpp"
#include "RenderStats.hpp"
#include "Color.hpp"
#include "RenderContext.hpp"
#include "VertexArray.hpp"
#include "robot2D/Core/WindowContext.hpp"

namespace robot2D {
    namespace priv {
        class RenderImpl;
    }

    /**
     * \brief Make possible to render either Robot2D's Drawables or render custom VertexArray.
     * \details It's usefull to render own ParticleEmitters, Custom Entites or etc ...
     *  Under hood setups BatchRender works with objects like quads because \n
     *  by default in 2D Space everything is simple to understand like quad.
     */
    class ROBOT2D_EXPORT_API RenderTarget {
    public:
        RenderTarget(const vec2u& size,
                     const WindowContext::RenderApi openGLVersion);
        virtual ~RenderTarget() = 0;

        /// Set new Viewport.
        virtual void setView(const View& view);

        /// get current Viewport.
        virtual const View& getView();

        /// get default viewport for Window.
        virtual const View& getDefaultView();

        /// Clear Window background space with custom color
        void clear(const Color& color = Color::Black);

        /// \brief Makes possible to draw your custom Drawable inherit.
        /// \details Also you are able to specify additional information.
        /// Check RenderStates to get more information.
        virtual void draw(const Drawable& drawable, const RenderStates& states
                                                    = RenderStates::Default);
        /// \brief Your custom Drawable inherit use it.
        /// \details Drawable use to specify how to draw itself. \n
        /// For example get Viewport or rendering stats.
        /// In this method you don't set your data because your drawable is quad \n
        /// In other case prefer other methods.
        /// Check RenderStates to get more information.
        virtual void draw(const RenderStates& states);

        /// \brief Your custom Drawable inherit use it.
        /// \details Drawable use to specify how to draw itself. \n
        /// For example get Viewport or rendering stats.
        /// In this method you don't set your data because your drawable is quad \n
        /// In other case prefer other methods.
        /// Check RenderStates to get more information.
        virtual void draw(const VertexData& data, const RenderStates& states);

        /// \brief Your custom Drawable inherit use it.
        /// \details Drawable use to specify how to draw itself. \n
        /// For example get Viewport or rendering stats.
        /// In this method you don't set your data because your drawable is quad \n
        /// In other case prefer other methods.
        /// Check RenderStates to get more information.
        virtual void draw(const VertexArray::Ptr& vertexArray, RenderStates states) const;

        virtual void beforeRender() const;
        virtual void afterRender() const;
        virtual void flushRender() const;

        /// Get BatchRender's Stats
        const RenderStats& getStats() const;
    private:
        void setup();
    protected:
        std::unique_ptr<priv::RenderImpl> m_render;
        vec2u m_size;
    };

}
