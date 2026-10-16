#pragma once

#include "WindowOptions.hpp"

namespace robot2D {
    /// Owns the style pushes from WindowOptions without retaining the options.
    class WindowStyleScope {
    public:
        explicit WindowStyleScope(const WindowOptions& options);
        WindowStyleScope(const WindowStyleScope&) = delete;
        WindowStyleScope& operator=(const WindowStyleScope&) = delete;
        WindowStyleScope(WindowStyleScope&&) = delete;
        WindowStyleScope& operator=(WindowStyleScope&&) = delete;
        ~WindowStyleScope();
    private:
        int m_variables;
        int m_colors;
    };

    /// Always ends Begin, including a collapsed/clipped window. Styles outlive End.
    /// Requires the creating ImGui context to remain current for this scope.
    class WindowScope {
    public:
        explicit WindowScope(const WindowOptions& options, bool* open = nullptr);
        WindowScope(const WindowScope&) = delete;
        WindowScope& operator=(const WindowScope&) = delete;
        WindowScope(WindowScope&&) = delete;
        WindowScope& operator=(WindowScope&&) = delete;
        ~WindowScope();
        explicit operator bool() const noexcept {
            return m_visible;
        }
    private:
        WindowStyleScope m_style;
        bool m_visible;
    };
}
