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

#include <robot2D/Core/Event.hpp>

namespace robot2D {

    template<>
    Event::SizeEvent Event::as() const {
        return size;
    }

    template<>
    Event::MouseMoveEvent Event::as() const {
        return move;
    }

    template<>
    Event::MouseButtonEvent Event::as() const {
        return mouse;
    }

    template<>
    Event::MouseWheelEvent Event::as() const {
        return wheel;
    }

    template<>
    Event::KeyboardEvent Event::as() const {
        return key;
    }

    template<>
    Event::TextEnterEvent Event::as() const {
        return text;
    }

    template<>
    Event::JoystickEvent Event::as() const {
        return joystick;
    }

}