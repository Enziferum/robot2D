#include <algorithm>
#include <cassert>
#include <exception>
#include <thread>
#include <GLFW/glfw3.h>
#include <robot2D/Core/DesktopRuntime.hpp>
#include "Desktop/DesktopWindowImpl.hpp"

namespace robot2D {
    namespace {
        DesktopRuntime* activeRuntime = nullptr;
        bool validJoystick(JoystickType joystick) {
            const int id = static_cast<int>(joystick);
            return id >= GLFW_JOYSTICK_1 && id <= GLFW_JOYSTICK_LAST;
        }
    }

    struct DesktopRuntime::Impl {
        Ownership ownership;
        std::thread::id thread = std::this_thread::get_id();
        std::vector<priv::DesktopWindowImpl*> windows;
        unsigned cursors = 0;
        GLFWjoystickfun previousJoystickCallback = nullptr;
    };

    WindowCreationError::WindowCreationError(WindowCreationStage stage, int nativeCode,
        const std::string& message) : std::runtime_error(message), m_stage(stage), m_nativeCode(nativeCode) {
    }

    DesktopRuntime::DesktopRuntime(Ownership ownership) : m_impl(std::make_unique<Impl>()) {
        if(activeRuntime) throw std::logic_error("A DesktopRuntime is already active");
        m_impl -> ownership = ownership;
        if(ownership == Ownership::Owned && !glfwInit()) {
            const char* description = nullptr;
            const int code = glfwGetError(&description);
            throw WindowCreationError(WindowCreationStage::Initialization, code,
                      std::string("GLFW initialization failed: ") + (description ? description : "unknown error"));
        }
        // glfwGetMonitors is valid without a window and diagnoses uninitialized borrowed GLFW.
        int count = 0;
        glfwGetError(nullptr);
        glfwGetMonitors(&count);
        const char* description = nullptr;
        const int code = glfwGetError(&description);
        if(code != GLFW_NO_ERROR) {
            const std::string message = description ? description : "GLFW unavailable";
            if(ownership == Ownership::Owned) glfwTerminate();
            throw WindowCreationError(WindowCreationStage::Initialization, code, message);
        }
        m_impl -> previousJoystickCallback = glfwSetJoystickCallback(joystickCallback);
        activeRuntime = this;
    }

    DesktopRuntime::~DesktopRuntime() noexcept {
        assert(m_impl -> thread == std::this_thread::get_id());
        assert(m_impl -> windows.empty() && m_impl -> cursors == 0);
        glfwSetJoystickCallback(m_impl -> previousJoystickCallback);
        activeRuntime = nullptr;
        if(m_impl -> ownership == Ownership::Owned) glfwTerminate();
    }

    void DesktopRuntime::checkThread() const {
        if(m_impl -> thread != std::this_thread::get_id())
            throw std::logic_error("DesktopRuntime must be used on its creating main thread");
    }

    void DesktopRuntime::attach(priv::DesktopWindowImpl& window) {
        checkThread();
        m_impl -> windows.push_back(&window);
    }

    void DesktopRuntime::detach(priv::DesktopWindowImpl& window) noexcept {
        auto& windows = m_impl -> windows;
        windows.erase(std::remove(windows.begin(), windows.end(), &window), windows.end());
    }

    // Keep the registry representation private to this translation unit.
    GLFWwindow* priv::DesktopWindowImpl::recoveryContext(const DesktopRuntime& desktop) noexcept {
        const auto& windows = desktop.m_impl -> windows;
        return windows.empty() ? nullptr : static_cast<GLFWwindow*>(windows.front() -> getRaw());
    }

    void DesktopRuntime::attachCursor() {
        checkThread(); ++m_impl -> cursors;
    }
    void DesktopRuntime::detachCursor() noexcept {
        --m_impl -> cursors;
    }

    void DesktopRuntime::processEvents() {
        checkThread();
        glfwPollEvents();
        std::exception_ptr firstError;
        for(auto* window : m_impl -> windows) {
            try { window -> rethrowCallbackError(); }
            catch(...) { if(!firstError) firstError = std::current_exception();}
        }
        if(firstError) std::rethrow_exception(firstError);
    }

    void DesktopRuntime::joystickCallback(int id, int action) noexcept {
        if(!activeRuntime) return;
        for(auto* window : activeRuntime -> m_impl -> windows)
            window -> joystickEvent(id, action);
    }

    bool DesktopRuntime::isJoystickAvailable(JoystickType joystickType) const {
        checkThread();
        return validJoystick(joystickType) && glfwJoystickPresent(static_cast<int>(joystickType)) == GLFW_TRUE;
    }

    bool DesktopRuntime::joystickIsGamepad(JoystickType joystickType) const {
        checkThread();
        return validJoystick(joystickType) && glfwJoystickIsGamepad(static_cast<int>(joystickType)) == GLFW_TRUE;
    }

    bool DesktopRuntime::getJoystickGamepadInput(JoystickType joystickType, JoystickGamepadInput& gamepadInput) const {
        checkThread();
        if(!validJoystick(joystickType)) return false;
        GLFWgamepadstate wgamepadstate;
        auto status = glfwGetGamepadState(static_cast<int>(joystickType), &wgamepadstate);
        if(status != GLFW_TRUE)
            return false;

        // TODO(a.raag): remove magic constraints //

        gamepadInput.leftManipulatorOffset = {
            wgamepadstate.axes[GLFW_GAMEPAD_AXIS_LEFT_X],
            wgamepadstate.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]
        };

        if(gamepadInput.leftManipulatorOffset.x == 1.5259022e-05f) {
            gamepadInput.leftManipulatorOffset.x = 0;
        }

        if(gamepadInput.leftManipulatorOffset.y == -1.5259022e-05f) {
            gamepadInput.leftManipulatorOffset.y = 0;
        }

        gamepadInput.rightManipulatorOffset = {
            wgamepadstate.axes[GLFW_GAMEPAD_AXIS_RIGHT_X],
            wgamepadstate.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]
        };

        if(gamepadInput.rightManipulatorOffset.x == 1.5259022e-05f) {
            gamepadInput.rightManipulatorOffset.x = 0;
        }

        if(gamepadInput.rightManipulatorOffset.y == -1.5259022e-05f) {
            gamepadInput.rightManipulatorOffset.y = 0;
        }

        gamepadInput.leftTrigger = wgamepadstate.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
        gamepadInput.rightTrigger = wgamepadstate.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];

        for(int i = 0; i < GLFW_GAMEPAD_BUTTON_LAST + 1; ++i)
            gamepadInput.buttons[i] = static_cast<bool>(wgamepadstate.buttons[i]);

        return true;
    }
}
