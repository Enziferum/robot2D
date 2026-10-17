#pragma once

#include <memory>
#include <stdexcept>
#include <robot2D/Config.hpp>
#include <robot2D/Core/Joystick.hpp>

namespace robot2D {
    namespace priv { class DesktopWindowImpl; }

    enum class WindowCreationStage { Initialization, Validation, NativeWindow, ContextLoader };

    /// A failed constructor publishes no window. Native diagnostics are copied, not borrowed.
    class ROBOT2D_EXPORT_API WindowCreationError : public std::runtime_error {
    public:
        WindowCreationError(WindowCreationStage stage, int nativeCode, const std::string& message);
        WindowCreationStage stage() const noexcept {
            return m_stage;
        }
        int nativeErrorCode() const noexcept {
            return m_nativeCode;
        }
    private:
        WindowCreationStage m_stage;
        int m_nativeCode;
    };

    /// Main-thread GLFW lifetime. Must outlive all its windows and native cursors.
    /// One runtime may be active. Borrowed mode requires externally initialized GLFW;
    /// the external owner must not terminate it until this runtime and its clients are gone.
    class ROBOT2D_EXPORT_API DesktopRuntime {
    public:
        enum class Ownership { Owned, Borrowed };
        explicit DesktopRuntime(Ownership ownership = Ownership::Owned);
        DesktopRuntime(const DesktopRuntime&) = delete;
        DesktopRuntime& operator=(const DesktopRuntime&) = delete;
        DesktopRuntime(DesktopRuntime&&) = delete;
        DesktopRuntime& operator=(DesktopRuntime&&) = delete;
        ~DesktopRuntime() noexcept;

        /// Pump all native windows once per frame; callback failures are rethrown here.
        void processEvents();
        bool isJoystickAvailable(JoystickType joystick) const;
        bool joystickIsGamepad(JoystickType joystick) const;
        bool getJoystickGamepadInput(JoystickType joystick, JoystickGamepadInput& input) const;
        /// Throws logic_error on use from a different thread. Destruction has the same precondition.
        void checkThread() const;
    private:
        friend class priv::DesktopWindowImpl;
        friend class Cursor;
        void attach(priv::DesktopWindowImpl& window);
        void detach(priv::DesktopWindowImpl& window) noexcept;
        void attachCursor();
        void detachCursor() noexcept;
        static void joystickCallback(int id, int action) noexcept;
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
