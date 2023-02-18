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
#include <string>
#include <robot2D/Config.hpp>

#include "Mouse.hpp"
#include "Keyboard.hpp"
#include "Joystick.hpp"

namespace robot2D {
    /**
     * \brief Describe OS-Based event to one format.
     */
    class ROBOT2D_EXPORT_API Event {
    public:
        enum EventType {
            Resized,
            Closed,
            GainFocus,
            LostFocus,
            MouseMoved,
            MouseWheel,
            MousePressed,
            MouseReleased,
            KeyPressed,
            KeyReleased,
            TextEntered,
            DragDrop,
            JoystickConnected,
            JoystickDisconnected,
            Count
        };

        EventType type;

        /// Window size
        struct SizeEvent {
            unsigned int widht;
            unsigned int heigth;
        };

        /// Mouse movement
        struct MouseMoveEvent {
            float x;
            float y;
        };

        /// Keyboard
        struct KeyboardEvent {
            Key code;
        };

        /// Mouse buttons
        struct MouseButtonEvent {
            enum buttons {
                left = 0,
                right = 1,
                middle = 2
            };

            int btn;
            int x;
            int y;
        };

        /// Mouse wheel
        struct MouseWheelEvent {
            float scroll_x;
            float scroll_y;
        };

        /// Text entering
        struct TextEnterEvent {
            unsigned int symbol;
        };

        /// Joystick
        struct JoystickEvent {
            int joytickId;
            bool isGamepad{false};
        };

        union {
            SizeEvent size;
            MouseMoveEvent move;
            MouseButtonEvent mouse;
            KeyboardEvent key;
            MouseWheelEvent wheel;
            TextEnterEvent text;
            JoystickEvent joystick;
        };

        /// \brief get event as concrete inside type
        template<typename T>
        T as() const;
    };
}