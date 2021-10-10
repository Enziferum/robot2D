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

#include <vector>
#include <string>
#include <robot2D/Config.hpp>
#include "Mouse.hpp"
#include "Keyboard.hpp"

namespace robot2D {
    /**
     * Describe OS-Based event to one format
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
            Count
        };

        EventType type;

        struct SizeEvent {
            unsigned int widht;
            unsigned int heigth;
        };

        struct MouseMoveEvent {
            float x;
            float y;
        };

        struct KeyboardEvent {
            Key code;
        };

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

        struct MouseWheelEvent {
            float scroll_x;
            float scroll_y;
        };

        struct TextEnterEvent {
            unsigned int symbol;
        };


        union {
            SizeEvent size;
            MouseMoveEvent move;
            MouseButtonEvent mouse;
            KeyboardEvent key;
            MouseWheelEvent wheel;
            TextEnterEvent text;
        };

    };
}