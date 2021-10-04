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

#include <robot2D/Core/Window.hpp>
#include <robot2D/Util/Logger.hpp>
#include "WindowImpl.hpp"

namespace robot2D {

    std::unique_ptr<priv::WindowImpl> Window::m_windowImpl = nullptr;

    Window::Window():
    m_win_size(800, 600),
    m_name("robot2D"),
    m_context(WindowContext::Default)
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


    Window::~Window() {}


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

    void Window::setTitle(const std::string& title) {
        m_windowImpl -> setTitle(title);
    }

    void Window::onResize(const int& w, const int& h) {
        //c++ hack before c++ 17 to set unused parameter
        (void)(w);
        //c++ hack before c++ 17 to set unused parameter
        (void)(h);
    }

    void Window::setIcon(std::vector<Texture>& icons) {
        m_windowImpl -> setIcon(icons);
    }

    WindowHandle Window::getRaw() const {
        return m_windowImpl -> getRaw();
    }

    void Window::create() {
        m_windowImpl = priv::WindowImpl::create(m_win_size, m_name, m_context);
        if(m_windowImpl == nullptr)
            throw std::runtime_error("Can't create WindowImpl");
    }

    const vec2u& Window::getSize() {
        return m_win_size;
    }


    // todo create
    bool Window::isMousePressed(const Mouse& button) {
        (void)button;
        return false;
    }

    bool Window::isKeyboardPressed(const Key& key) {
        return m_windowImpl ->isKeyboardPressed(key);
    }

    float Window::getDeltaTime() {
        return m_windowImpl -> getDeltaTime();
    }

    void Window::setDrapDropCallback(Window::DrapDropCallback&& callback) {
        m_windowImpl -> setDrapDropCallback(std::move(callback));
    }

    void Window::setMouseCursorVisible(const bool& flag) {
        m_windowImpl ->setMouseCursorVisible(flag);
    }


}