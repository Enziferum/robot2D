#include <robot2D/Core/Joystick.hpp>
#include <robot2D/Core/Window.hpp>

namespace robot2D {
    bool isJoystickAvailable(const JoystickType& joystickType) {
        return Window::isJoystickAvailable(joystickType);
    }

    bool joystickIsGamepad(const JoystickType &joystickType) {
        return Window::JoystickIsGamepad(joystickType);
    }

    bool getJoystickGamepadInput(const JoystickType& joystickType, JoystickGamepadInput& gamepadInput) {
        return Window::getJoystickGamepadInput(joystickType, gamepadInput);
    }
}