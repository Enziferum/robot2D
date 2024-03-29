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
#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Util/Logger.hpp>

namespace robot2D{

    RenderWindow::RenderWindow():
            Window{}, RenderTarget{m_win_size,
                                   m_context.renderApi ,
                                   m_context.renderDimensionType} {
    }

    RenderWindow::RenderWindow(const vec2u& size,
                               const std::string& name,
                               const WindowContext& context):
                               Window(size, name, context),
                               RenderTarget(m_win_size,
                                            context.renderApi,
                                            m_context.renderDimensionType) {
    }


    void RenderWindow::onResize(const int& width, const int& height) {
        m_size = vec2u(width, height);
    }

    void RenderWindow::resize(const vec2i& newSize) {
        m_size = vec2u { static_cast<unsigned int>(newSize.x),
                         static_cast<unsigned int>(newSize.y)};
        m_win_size = m_size;
    }
}