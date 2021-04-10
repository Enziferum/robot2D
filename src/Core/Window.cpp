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

#include <robot2D/Core/Window.h>
#include <robot2D/Util/Logger.h>
#include "WindowImpl.hpp"

namespace robot2D {

    Window::Window():
    m_windowImpl(nullptr),
    m_win_size(800, 600),
    m_name("robot2D"),
    m_context()
    {
        Window::create();
    }

    Window::Window(const vec2u& size, const std::string& name, WindowContext& context):
            m_win_size(size.x, size.y),
            m_name(name),
            m_context(context)
            {
        Window::create();
    }


    Window::~Window() {
        if(m_windowImpl != nullptr) {
            delete m_windowImpl;
            m_windowImpl = nullptr;
        }
    }


    bool Window::isOpen() const {
        return m_windowImpl -> isOpen();
    }

    bool Window::pollEvents(Event& event) {
        if(m_windowImpl == nullptr)
            return false;

        if(m_windowImpl -> pollEvents(event)) {
            if(event.type == Event::Resized)
                onResize(event.size.widht, event.size.heigth);
            return true;
        }

        return false;
    }

    void Window::clear(const Color& color) {
       m_windowImpl -> clear(color);
    }

    void Window::display() {
        m_windowImpl -> display();
    }

    void Window::close() {
        m_windowImpl -> close();
    }


    void Window::onResize(const int &w, const int &h) {
        //c++ hack before c++ 17 to set unused parameter
        (void)(w);
        //c++ hack before c++ 17 to set unused parameter
        (void)(h);
    }


    void Window::setIcon(std::vector<Texture>& icons) {
        m_windowImpl -> setIcon(icons);
    }

    void* Window::get_RawWindow() const {
        return m_windowImpl -> get_RawWindow();
    }

    void Window::create() {
        m_windowImpl = priv::WindowImpl::create(m_win_size, m_name, m_context);
        if(m_windowImpl == nullptr) {
            //todo throw error
        }
    }

    const vec2u& Window::get_size() {
        return m_win_size;
    }

    bool Window::getMouseButton(const int &button) {
    //    return glfwGetMouseButton(m_window, button);
    }

    void Window::setCursorPosition(const vec2f &pos) {
     //   glfwSetCursorPos(m_window, pos.x, pos.y);
    }

    vec2f Window::getCursorPos() {

//        vec2f pos;
//        double x, y;
//        glfwGetCursorPos(m_window, &x, &y);
//        pos.x = x; pos.y = y;
//        return pos;
    }

}