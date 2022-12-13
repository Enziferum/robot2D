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
#include "Matrix3D.hpp"

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
                     const WindowContext::RenderApi openGLVersion,
                     const WindowContext::RenderDimensionType renderDimensionType);
        virtual ~RenderTarget() = 0;

        /// Set new Viewport.
        virtual void setView(const View& view, unsigned int layerID = 1);

        virtual void setView3D(const Matrix3D& projection, const Matrix3D& view);

        /// get current Viewport.
        virtual const View& getView(unsigned int layerID = 1) const;

        /// get default viewport for Window.
        virtual const View& getDefaultView();

        /// Clear Window background space with custom color
        void clear(const Color& color = Color::Black);

        /// Adding Layer inside Render Impl
        void createLayer();

        /// Get actual layer count
        unsigned int getLayerCount() const;

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

        virtual void draw(const Vertex3DData& data, const RenderStates& states);

        /// \brief Your custom Drawable inherit use it.
        /// \details Drawable use to specify how to draw itself. \n
        /// For example get Viewport or rendering stats.
        /// In this method you don't set your data because your drawable is quad \n
        /// In other case prefer other methods.
        /// Check RenderStates to get more information.
        virtual void draw(const VertexArray::Ptr& vertexArray, RenderStates states) const;


        virtual void draw3D(const VertexArray::Ptr& vertexArray, RenderStates states) const;

        /// \brief Prepare render to current frame render
        virtual void beforeRender() const;

        /// \brief Processing render of current frame render.
        virtual void afterRender() const;

        /// \brief Direct render of current frame render.
        virtual void flushRender(unsigned int layerID) const;

        /// \brief allow to render one drawable in one call without before / after / flush ? .
        /// \details don't use several render function to render several Drawables. \n
        /// Use instead berofeRender -> n - times draw() afterRender.
        /// Don't afraid n - time draws calls. draw function = pack into BatchRender.
        virtual void render(const Drawable& drawable,
                            const RenderStates& states = RenderStates::Default);

        /// Get BatchRender's Stats
        const RenderStats& getStats() const;


        vec2f mapPixelToCoords(const vec2i& point, const View& view, unsigned int layerID = 1) const;

        vec2f mapPixelToCoords(const vec2i& point, unsigned int layerID = 1) const;
    private:
        void setup();
    protected:
        std::unique_ptr<priv::RenderImpl> m_render;
        vec2u m_size;
    };

}
