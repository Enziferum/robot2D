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

#include "DesktopWindowImpl.hpp"

namespace robot2D {
    namespace priv {

        DesktopWindowImpl::DesktopWindowImpl() {

        }

        DesktopWindowImpl::~DesktopWindowImpl() {

        }

        bool DesktopWindowImpl::pollEvents(Event& event) {
            return false;
        }

        void* DesktopWindowImpl::get_RawWindow() {
            return static_cast<void*>(m_window);
        }

        void DesktopWindowImpl::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {

        }

        void DesktopWindowImpl::cursor_callback(GLFWwindow *window, double xpos, double ypos) {

        }

        void DesktopWindowImpl::mouseWhell_callback(GLFWwindow *window, double xpos, double ypos) {

        }

        void DesktopWindowImpl::mouse_callback(GLFWwindow *window, int key, int action, int mods) {

        }

        void DesktopWindowImpl::size_callback(GLFWwindow *window, int w, int h) {

        }

        void DesktopWindowImpl::view_callback(GLFWwindow *window, int w, int h) {

        }

        void DesktopWindowImpl::maximized_callback(GLFWwindow *window, int state) {

        }


    }
}