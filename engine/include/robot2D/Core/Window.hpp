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
#include <queue>
#include <string>
#include <vector>
#include <memory>

#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Graphics/Image.hpp>

#include "Vector2.hpp"
#include "Event.hpp"
#include "WindowContext.hpp"
#include "Cursor.hpp"

namespace robot2D {

    namespace priv {
        class WindowImpl;
    }

    using WindowHandle = void*;

    /**
     * \brief Creates Window where you can draw own entites / objects.
     */
    class ROBOT2D_EXPORT_API Window {
    public:
        ////////////////////////////////////////////////////////////////
        /// \brief Creates default Window object with default options.
        ///
        /// To specify custom options use other Constructors.
        ///
        ////////////////////////////////////////////////////////////////
        Window();
        ////////////////////////////////////////////////////////////////
        /// \brief Special Constructor. Allow to specify options.
        ///
        /// \param size Specify window size in 2D Space
        /// \param name Specify custom name or use setTitle method.
        /// \param windowContext Specify options to Window. Check WindowContext for detail information.
        ///
        ////////////////////////////////////////////////////////////////
        Window(const vec2u& size,
               const std::string& name,
               const WindowContext& windowContext = robot2D::WindowContext::Default);
        ~Window();

        ////
        //// \brief process OS events and puts it to robot2D::Event
        ////
        bool pollEvents(Event& event);

        ////
        //// \brief check if window is opened just now
        ////
        bool isOpen() const;

        ////
        /// \brief process window closing, in next version will allow to set onClose callback
        ///
        void close();

        ///
        /// \brief make swap buffers in render context
        ///
        void display();

        void setTitle(const std::string& title);

        void setSize(const robot2D::vec2u& size);

        const vec2u& getSize();

        /// \brief allow you to set big/small icons to your app
        void setIcon(robot2D::Image&& icon);

        WindowHandle getRaw() const;

        void setCursor(const Cursor& cursor);

        void setMouseCursorVisible(const bool& flag);

        ////// Window manipulate options //////

        robot2D::vec2u getMonitorSize() const;
        void setPosition(const robot2D::vec2u& position);
        robot2D::vec2u getPosition() const;

        void setResizable(const bool& flag);
        void setMaximazed(const bool& flag);
        void setVsync(bool flag);

        ////// Window manipulate options //////


        void setMousePos(const vec2f& pos);
        vec2f getMousePos() const;

        static bool isMousePressed(const Mouse& button);
        static bool isKeyboardPressed(const Key& key);
        static bool isJoystickAvailable(const JoystickType& joystickType);
        static bool JoystickIsGamepad(const JoystickType& joystickType);

        static bool getJoystickGamepadInput(const JoystickType& joystickType, JoystickGamepadInput& gamepadInput);
    protected:
        virtual void onResize(const int& w, const int& h);
    private:
        void create();
    protected:
        static std::unique_ptr<priv::WindowImpl> m_windowImpl;

        /// window settings
        vec2u m_win_size;
        std::string m_name;
        WindowContext m_context;
    };
}