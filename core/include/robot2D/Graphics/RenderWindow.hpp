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

#include <robot2D/Core/Window.hpp>
#include "RenderTarget.hpp"

namespace robot2D {

    /**
     * \brief RenderWindow is mix of Window Management and Render Stuff
     */
    class ROBOT2D_EXPORT_API RenderWindow : public Window, public RenderTarget {
    public:
        explicit RenderWindow(DesktopRuntime& desktop);
        RenderWindow(DesktopRuntime& desktop, const vec2u& size, const std::string& name,
            const WindowContext& context = robot2D::WindowContext::Default);
        RenderWindow(const RenderWindow&) = delete;
        RenderWindow(const RenderWindow&&) = delete;
        RenderWindow& operator=(const RenderWindow&) = delete;
        RenderWindow& operator=(const RenderWindow&&) = delete;
        ~RenderWindow() override;

        /// Compatibility explicit 1:1 render sizing; does not resize the native window.
        void resize(const robot2D::vec2i& newSize);
        /// Select this context and synchronize the native output before rendering to it.
        /// Returns false for minimized/zero targets; does not pause updates or pump events.
        bool synchronizeSize();
        /// Same synchronization with a supplied platform snapshot, useful for embedded surfaces.
        bool synchronizeSize(const WindowMetrics& metrics);
        /// Map logical client coordinates through this window's current physical viewport.
        vec2f mapClientToCoords(const vec2f& point, const View& view, unsigned int layerID = 1) const;
    };
}