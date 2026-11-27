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
#include <stdexcept>
#include <limits>
#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Util/Logger.hpp>
#include "RenderImpl.hpp"

namespace robot2D {

    RenderWindow::RenderWindow(DesktopRuntime& desktop) :
        Window{desktop}, RenderTarget{m_win_size,
            m_context.renderApi,
            m_context.renderDimensionType} {
        synchronizeSize();
    }

    RenderWindow::RenderWindow(DesktopRuntime& desktop, const vec2u& size,
        const std::string& name,
        const WindowContext& context) :
        Window(desktop, size, name, context),
        RenderTarget(m_win_size,
            context.renderApi,
            m_context.renderDimensionType) {
        synchronizeSize();
    }


    RenderWindow::~RenderWindow() {
        makeCurrent();
    }

    bool RenderWindow::synchronizeSize() {
        return synchronizeSize(getMetrics());
    }

    bool RenderWindow::synchronizeSize(const WindowMetrics& metrics) {
        if(!metrics.drawable()) return false;
        const auto limit = static_cast<unsigned>(std::numeric_limits<int>::max());
        if(metrics.logicalSize.x > limit || metrics.logicalSize.y > limit ||
            metrics.framebufferSize.x > limit || metrics.framebufferSize.y > limit)
            throw std::invalid_argument("RenderWindow metrics exceed the native dimension range");
        makeCurrent();
        m_render -> setSurfaceSize(metrics.logicalSize, metrics.framebufferSize);
        m_size = metrics.logicalSize;
        glViewport(0, 0, static_cast<int>(metrics.framebufferSize.x), static_cast<int>(metrics.framebufferSize.y));
        return true;
    }

    vec2f RenderWindow::mapClientToCoords(const vec2f& point, const View& view, unsigned int layerID) const {
        (void)layerID;
        const auto metrics = getMetrics();
        if(!metrics.drawable()) return view.getCenter();
        const auto viewport = m_render -> getViewport(view);
        if(!viewport.width || !viewport.height) return view.getCenter();
        const vec2f pixels{point.x * metrics.framebufferSize.x / metrics.logicalSize.x,
                           point.y * metrics.framebufferSize.y / metrics.logicalSize.y};
        const vec2f normalized{-1.f + 2.f * (pixels.x - viewport.lx) / viewport.width,
                               1.f - 2.f * (pixels.y - viewport.ly) / viewport.height};
        return view.getInverseTransform().transformPoint(normalized);
    }

    void RenderWindow::resize(const vec2i& newSize) {
        if(newSize.x <= 0 || newSize.y <= 0)
            return;
        m_render -> setSize({static_cast<unsigned int>(newSize.x), static_cast<unsigned int>(newSize.y)});
        m_size = vec2u { static_cast<unsigned int>(newSize.x),
                         static_cast<unsigned int>(newSize.y)};
    }
}