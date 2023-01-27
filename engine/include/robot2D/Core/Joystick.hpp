/*********************************************************************
(c) Alex Raag 2022
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
#include <cstdint>
#include <array>

#include <robot2D/Core/Vector2.hpp>

namespace robot2D {
    enum class JoystickType: std::int32_t {
        One = 0, Two, Three, Four
    };

    enum class JoystickGamepadButton {
        ButtonA = 0,
        ButtonB,
        ButtonX,
        ButtonY,
        ButtonLeftBumper,
        ButtonRightBumper,
        ButtonBack,
        ButtonStart,
        ButtonGuide,
        ButtonLeftThumb,
        ButtonRightThumb,
        ButtonDpadUp,
        ButtonDpadRight,
        ButtonDpadDown,
        ButtonDpadLeft,
        ButtonLast,
        ButtonCross=ButtonA,
        ButtonCircle=ButtonB,
        ButtonSquare=ButtonX,
        ButtonTriangle=ButtonY,
    };

    struct JoystickGamepadInput {
        // from -1 to 1 //
        robot2D::vec2f leftManipulatorOffset;
        robot2D::vec2f rightManipulatorOffset;

        std::array<bool, 15> buttons;

        float leftTrigger{-2.F};
        float rightTrigger{-2.F};
    };

    bool isJoystickAvailable(const JoystickType& joystickType);

    bool joystickIsGamepad(const JoystickType& joystickType);

    bool getJoystickGamepadInput(const JoystickType& joystickType, JoystickGamepadInput& gamepadInput);
}