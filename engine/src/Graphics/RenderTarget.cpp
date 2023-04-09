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
#include <cassert>

#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Graphics/Rect.hpp>

#include "RenderImpl.hpp"
#include "robot2D/Graphics/RenderAPI.hpp"

namespace robot2D {

    RenderTarget::RenderTarget(const vec2u& size,
                               const WindowContext::RenderApi openGLVersion,
                               const WindowContext::RenderDimensionType renderDimensionType):
    m_render(nullptr),
    m_size(size) {
        if(openGLVersion == WindowContext::RenderApi::OpenGL3_3)
            RenderAPI::m_api = RenderApi::OpenGL3_3;
        else if(openGLVersion == WindowContext::RenderApi::OpenGL4_5)
            RenderAPI::m_api = RenderApi::OpenGL4_5;

        switch (renderDimensionType) {
            case WindowContext::RenderDimensionType::TwoD:
                RenderAPI::m_dimensionType = RenderDimensionType::TwoD;
                break;
            case WindowContext::RenderDimensionType::ThreeD:
                RenderAPI::m_dimensionType = RenderDimensionType::ThreeD;
                break;
            case WindowContext::RenderDimensionType::Both:
                RenderAPI::m_dimensionType = RenderDimensionType::Both;
                break;
        }

        setup();
    }

    RenderTarget::~RenderTarget() {}

    void RenderTarget::setup() {
        if(!m_render)
            m_render = robot2D::priv::RenderImpl::create(m_size);

        assert(m_render != nullptr && "Render Impl must be not null");

        m_render -> setup();
    }

    void RenderTarget::clear(const Color& color) {
        m_render -> clear(color);
    }

    void RenderTarget::draw(const Drawable& drawable, const RenderStates& states) {
        drawable.draw(*this, states);
    }

    void RenderTarget::draw(const RenderStates &states) {
        m_render -> render(states);
    }

    void RenderTarget::draw(const VertexData& data, const RenderStates& states) {
        if(!m_render)
            return;

        m_render -> render(data, states);
    }

    void RenderTarget::draw(const Vertex3DData& data, const RenderStates& states) {
        if(!m_render)
            return;

        m_render -> render(data, states);
    }

    void RenderTarget::setView(const View& view, unsigned int layerID) {
        m_render -> setView(view, layerID);
    }

    const View& RenderTarget::getView(unsigned int layerID) const {
        return m_render -> getView(layerID);
    }

    const View& RenderTarget::getDefaultView() {
        return m_render -> getDefaultView();
    }

    void RenderTarget::beforeRender() const {
        m_render -> beforeRender();
    }

    void RenderTarget::afterRender() const {
        m_render -> afterRender();
    }

    void RenderTarget::flushRender(unsigned int layerID) const {
        m_render -> flushRender(layerID);
    }

    const RenderStats& RenderTarget::getStats() const {
        return m_render -> getStats();
    }

    void RenderTarget::draw(const VertexArray::Ptr& vertexArray, RenderStates states) const {
        if(m_render == nullptr)
            return;
        m_render -> render(vertexArray, states);
    }

    void RenderTarget::render(const Drawable& drawable, const RenderStates& states) {
        if(m_render == nullptr)
            return;
        beforeRender();
        drawable.draw(*this, states);
        afterRender();
    }

    void RenderTarget::createLayer() {
        m_render -> createLayer();
    }

    unsigned int RenderTarget::getLayerCount() const {
        return m_render -> getLayerCount();
    }

    vec2f RenderTarget::mapPixelToCoords(const vec2i& point, const View &view, unsigned int layerID) const {
        // First, convert from viewport coordinates to homogeneous coordinates
        vec2f normalized;
        auto port = m_render -> getViewport(view);
        FloatRect viewport = {port.lx, port.ly, port.width, port.height};
        normalized.x       = -1.f + 2.f * (static_cast<float>(point.x) - viewport.lx) / viewport.width;
        normalized.y       = 1.f - 2.f * (static_cast<float>(point.y) - viewport.ly) / viewport.height;

        // Then transform by the inverse of the view matrix
        return view.getInverseTransform().transformPoint(normalized);
    }

    vec2f RenderTarget::mapPixelToCoords(const vec2i& point, unsigned int layerID) const {
        return mapPixelToCoords(point, getView(layerID), layerID);
    }

    void RenderTarget::setView3D(const Matrix3D& projection, const Matrix3D& view) {
        m_render -> setView3D(projection, view);
    }

    void RenderTarget::draw3D(const VertexArray::Ptr& vertexArray, RenderStates states) const {
        m_render -> render3D(vertexArray, states);
    }

    void RenderTarget::setRawView(float *rawMatrix) {
        m_render -> setRawView(rawMatrix);
    }

}