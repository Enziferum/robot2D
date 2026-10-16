/*********************************************************************
   (c) Alex Raag 2024
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

#include <array>
#include <memory>

#include <imgui/imgui.h>
#include <robot2D/Core/Window.hpp>
#include "GuiFontConfig.hpp"
#include "InputEvents.hpp"

namespace robot2D {
    namespace priv { class GuiImpl; }

    struct GuiConfiguration {
        Window* window = nullptr; ///< Borrowed; must outlive shutdown.
        std::vector<GuiFontConfig> fonts;
        FontId defaultFont = 0;
        bool allowDefaultFallback = false;
    };

    enum class GuiState { Uninitialized, Ready, BuildingFrame, DrawDataReady };

    /// Select an explicitly owned context for non-frame configuration/settings calls.
    class GuiContextScope {
    public:
        explicit GuiContextScope(ImGuiContext* context) : m_previous(ImGui::GetCurrentContext()) {
            ImGui::SetCurrentContext(context);
        }
        ~GuiContextScope() {
            ImGui::SetCurrentContext(m_previous);
        }
        GuiContextScope(const GuiContextScope&) = delete;
        GuiContextScope& operator=(const GuiContextScope&) = delete;
    private:
        ImGuiContext* m_previous;
    };

    /// One UI thread. Explicit shutdown must precede destruction of the GL context/window.
    class Gui {
    public:
        Gui();
        Gui(const Gui&) = delete;
        Gui& operator=(const Gui&) = delete;
        Gui(Gui&&) = delete;
        Gui& operator=(Gui&&) = delete;
        ~Gui() noexcept;

        bool initialize(const GuiConfiguration& configuration);
        void shutdown() noexcept;
        void handleEvents(const Event& event);
        GuiInputCapture inputCapture() const;
        bool beginFrame(float deltaSeconds);
        bool endFrame();
        bool cancelFrame();
        /// Consumes DrawDataReady even on failure or a minimized target. Does not clear/swap.
        bool render();
        GuiState state() const noexcept;
        const std::string& lastError() const noexcept;
        ImGuiContext* context() const noexcept;
        /// Valid only until shutdown; use FontId rather than retaining the returned pointer.
        ImFont* font(FontId id) const noexcept;
    private:
        std::unique_ptr<priv::GuiImpl> m_impl;
    };
}
