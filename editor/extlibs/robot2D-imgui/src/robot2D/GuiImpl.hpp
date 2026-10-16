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
#include <robot2D/imgui/Gui.hpp>
#include "Render.hpp"
#include "FontSet.hpp"

namespace robot2D::priv {
    class GuiImpl {
    public:
        ~GuiImpl() noexcept;
        bool initialize(const GuiConfiguration& configuration);
        void shutdown() noexcept;
        void handleEvents(const Event& event);
        GuiInputCapture inputCapture() const;
        bool beginFrame(float dt);
        bool endFrame();
        bool cancelFrame();
        bool render();
        GuiState state() const noexcept {
            return m_state;
        }
        const std::string& lastError() const noexcept {
            return m_error;
        }
        ImGuiContext* context() const noexcept {
            return m_context;
        }
        ImFont* font(FontId id) const noexcept {
            return m_fonts.get(id);
        }
    private:
        bool fail(const std::string& reason);
        void updateMouseCursor();
        Window* m_window = nullptr;
        ImGuiContext* m_context = nullptr;
        ImGuiContext* m_previous = nullptr;
        Cursor m_defaultCursor; // Empty native cursor; shutdown must not allocate.
        std::array<std::unique_ptr<Cursor>, ImGuiMouseCursor_COUNT> m_cursors;
        bool m_windowHasFocus = true;
        GuiState m_state = GuiState::Uninitialized;
        std::string m_error;
        FontSet m_fonts;
        GuiRender m_render;
    };
}
