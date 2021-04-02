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


#include <robot2D/Graphics/GL.h>
#include "robot2D/Graphics/RenderWindow.h"

namespace robot2D{

    RenderWindow::RenderWindow():
    Window(),
    RenderTarget(m_win_size)
    {
    }

    RenderWindow::RenderWindow(const vec2u &size, const std::string &name, const bool &vsync)
    : Window(size, name,vsync),
      RenderTarget(m_win_size) {
    }

    RenderWindow::~RenderWindow() {

    }

    //todo applyView function
    void RenderWindow::onResize(const int &w, const int &h) {
        int x, y;
        glfwGetFramebufferSize(m_window, &x, &y);

//        if(w != x || h != y)
//            std::cout << "not valid framebuffer size" << std::endl;

        m_size = vec2u(w, h);
        ortho_projection(mat, 0.0f, static_cast<float>(m_size.x),
                         static_cast<float>(m_size.y),
                         0.0f, -1.0f, 1.0f);

        m_spriteShaders.use();
        m_spriteShaders.set_parameter("projection", &mat.mat[0][0]);

        glViewport(0, 0, w, h);
    }

}