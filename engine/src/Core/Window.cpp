/*********************************************************************
(c) Alex Raag 2023
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

    Window::Window(const vec2u& size, const std::string& name, const WindowContext& context):
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

    void Window::setIcon(robot2D::Image&& icon) {
        m_windowImpl -> setIcon(std::move(icon));
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

    bool Window::isMousePressed(const Mouse& button) {
        return m_windowImpl -> isMousePressed(button);
    }

    bool Window::isKeyboardPressed(const Key& key) {
        return m_windowImpl -> isKeyboardPressed(key);
    }

    bool Window::isJoystickAvailable(const JoystickType& joystickType) {
        return m_windowImpl -> isJoystickAvailable(joystickType);
    }

    bool Window::JoystickIsGamepad(const JoystickType& joystickType) {
        return m_windowImpl -> JoystickIsGamepad(joystickType);
    }

    void Window::setMouseCursorVisible(const bool& flag) {
        m_windowImpl -> setMouseCursorVisible(flag);
    }

    void Window::setCursor(const Cursor& cursor) {
        m_windowImpl -> setCursor(cursor);
    }

    robot2D::vec2u Window::getMonitorSize() const {
        if(!m_windowImpl) {
            return {};
        }
        return m_windowImpl -> getMonitorSize();
    }

    void Window::setPosition(const vec2u& position) {
        if(!m_windowImpl)
            return;
        m_windowImpl -> setPosition(position);
    }

    robot2D::vec2u Window::getPosition() const {
        if(!m_windowImpl) {
            return {};
        }
        return m_windowImpl -> getPosition();
    }

    void Window::setSize(const vec2u& size) {
        m_win_size = size;
        m_windowImpl -> setSize(size);
    }

    void Window::setResizable(const bool& flag) {
        m_windowImpl -> setResizable(flag);
    }

    void Window::setMaximazed(const bool &flag) {
        m_windowImpl -> setMaximazed(flag);
    }

    void Window::setMousePos(const vec2f& pos) {
        m_windowImpl -> setMousePos(pos);
    }

    vec2f Window::getMousePos() const {
        return m_windowImpl -> getMousePos();
    }

    bool Window::getJoystickGamepadInput(const JoystickType& joystickType, JoystickGamepadInput& gamepadInput) {
        return m_windowImpl -> getJoystickGamepadInput(joystickType, gamepadInput);
    }

    void Window::setVsync(bool flag) {
        m_windowImpl -> setVsync(flag);
    }

    void Window::addDropCallback(std::function<void(std::vector<std::string>&&)>&& callback) {
        m_windowImpl -> addDropCallback(std::move(callback));
    }

}