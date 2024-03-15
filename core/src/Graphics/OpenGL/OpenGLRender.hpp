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

#include <unordered_map>
#include <vector>

#include <robot2D/Graphics/RenderStates.hpp>
#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Graphics/View.hpp>
#include <robot2D/Graphics/RenderStats.hpp>
#include <robot2D/Graphics/RenderAPI.hpp>

#include "../RenderImpl.hpp"
#include "RenderBuffer.hpp"
#include "RenderLayer.hpp"

namespace robot2D {
    using uint = unsigned int;

    namespace priv {

        class ROBOT2D_EXPORT_API OpenGLRender: public RenderImpl {
        public:
            OpenGLRender(const vec2u& windowSize);
            ~OpenGLRender();

            void setup() override;
            void createLayer() override;
            unsigned int getLayerCount() const  override;

            void clear(const Color &color = Color::Black) override;

            void render(const RenderStates& states) override;
            void render(const VertexData& data, const RenderStates& states) const override;
            void render(const Vertex3DData& data, const RenderStates& states) const override;
            void render(const VertexArray::Ptr& vertexArray, RenderStates states) const override;
            void render3D(const VertexArray::Ptr& vertexArray, RenderStates states) const override;

            void setView(const View& view, unsigned int layerID) override;
            void setView3D(const Matrix3D& projection, const Matrix3D& view) override;
            void setRawView(float* rawMatrix) override;

            /// 2D Render View ///
            const View& getView(unsigned int layerID) override;
            const View& getDefaultView() override;
            /// 2D Render View ///

            virtual const RenderStats& getStats() const override;
        private:
            void setupOpenGL();
            void destroy();

            virtual void beforeRender() const override;
            virtual void afterRender()const override;
            virtual void flushRender(unsigned int layerID) const override;

            IntRect getViewport(const View& view) override;

            void applyCurrentView(unsigned int layerID);

            void setupLayer();
            void renderCache(unsigned int layerID) const;
        private:
            mutable std::vector<RenderLayer> m_renderLayers;
            View m_default;
            mutable RenderStats m_stats;
            RenderApi m_renderApi;
            RenderDimensionType m_dimensionType;

            enum class ShaderKey {
                TextureSamples,
                Projection,
                is3DRender,
                View
            };

            /// Instead using raw text in shader better have correct setup map
            std::unordered_map<ShaderKey, std::string> m_shaderKeys;
        };
    }
}
