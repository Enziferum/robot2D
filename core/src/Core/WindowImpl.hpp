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

#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <string>

#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Graphics/Image.hpp>

#include <robot2D/Core/Event.hpp>
#include <robot2D/Core/WindowContext.hpp>
#include <robot2D/Core/Cursor.hpp>

namespace robot2D::priv {
    class WindowImpl {
    protected:
        using Ptr = std::unique_ptr<WindowImpl>;
    public:
        WindowImpl();
        virtual ~WindowImpl() = 0;

        virtual bool pollEvents(Event& event) = 0;
        static WindowImpl::Ptr create();
        static WindowImpl::Ptr create(const robot2D::vec2u& size,
                                      const std::string& name, WindowContext& context);

        virtual void* getRaw() = 0;

        virtual bool isOpen() const = 0;
        virtual void setTitle(const std::string& title) const = 0;
        virtual void close() = 0;
        virtual void display() = 0;
        virtual bool isMousePressed(const Mouse& button) = 0;
        virtual bool isKeyboardPressed(const Key& key) = 0;
        virtual bool isJoystickAvailable(const JoystickType& joystickType) = 0;
        virtual bool JoystickIsGamepad(const JoystickType& joystickType) = 0;
        virtual bool getJoystickGamepadInput(const JoystickType& joystickType,
                                             JoystickGamepadInput& gamepadInput) = 0;

        virtual void setSize(const robot2D::vec2u& size) = 0;

        virtual void setIcon(robot2D::Image&& icon) = 0;
        virtual void setMouseCursorVisible(const bool& flag) = 0;
        virtual void setCursor(const Cursor& cursor) = 0;

        virtual robot2D::vec2u getMonitorSize() const = 0;
        virtual void setPosition(const robot2D::vec2u& position) = 0;
        virtual robot2D::vec2u getPosition() const = 0;
        virtual void setResizable(const bool& flag) = 0;
        virtual void setMaximazed(const bool& flag) = 0;

        virtual void setMousePos(const vec2f& pos) = 0;
        virtual vec2f getMousePos() const = 0;
        virtual void setVsync(bool flag) = 0;

        virtual void addDropCallback(std::function<void(std::vector<std::string>&&)>&& callback) = 0;
    };
}