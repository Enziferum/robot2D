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

#include "GuiImpl.hpp"
#include <robot2D/Util/Logger.hpp>
#include <cmath>
#include <exception>

namespace robot2D::priv {
    GuiImpl::~GuiImpl() noexcept {
        shutdown();
    }

    bool GuiImpl::fail(const std::string& reason) {
        m_error = reason;
        RB_EDITOR_ERROR("[GUI] {}", reason);
        return false;
    }

    bool GuiImpl::initialize(const GuiConfiguration& configuration) {
        if(m_state != GuiState::Uninitialized) return fail("initialize rejected: GUI already initialized");
        if(!configuration.window ||
            !configuration.window -> getRaw()) return fail("initialize requires a live window/context");
        m_error.clear();
        auto* previous = ImGui::GetCurrentContext();
        try {
            m_window = configuration.window;
            m_context = ImGui::CreateContext();
            ImGui::SetCurrentContext(m_context);
            auto& io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.BackendPlatformName = "ImGui_robot2D";
            io.BackendRendererName = "robot2D_OpenGL";
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            if(!m_fonts.build(configuration, m_error) || !m_render.initialize()) {
                if(m_error.empty()) m_error = m_render.lastError();
                shutdown(); ImGui::SetCurrentContext(previous); return fail(m_error);
            }
            for(auto& cursor : m_cursors) cursor = std::make_unique<Cursor>(m_window -> desktop());
            m_cursors[ImGuiMouseCursor_Arrow] -> createDefault();
            m_cursors[ImGuiMouseCursor_TextInput] -> create(CursorType::TextInput);
            m_cursors[ImGuiMouseCursor_ResizeNS] -> create(CursorType::ResizeUpDown);
            m_cursors[ImGuiMouseCursor_ResizeEW] -> create(CursorType::ResizeLeftRight);
            m_cursors[ImGuiMouseCursor_Hand] -> create(CursorType::Hand);
            bool cursorsReady = true;
            for(int cursor : {ImGuiMouseCursor_Arrow, ImGuiMouseCursor_TextInput, ImGuiMouseCursor_ResizeNS,
                              ImGuiMouseCursor_ResizeEW,
                              ImGuiMouseCursor_Hand}) cursorsReady &= m_cursors[cursor] -> getRaw() != nullptr;
            if(cursorsReady) io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
            io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
            m_windowHasFocus = true;
            m_state = GuiState::Ready;
            ImGui::SetCurrentContext(previous);
            return true;
        }
        catch(const std::exception& error) {
            shutdown(); ImGui::SetCurrentContext(previous);
            return fail(std::string("initialize failed: ") + error.what());
        }
        catch(...) {
            shutdown(); ImGui::SetCurrentContext(previous); throw;
        }
    }

    void GuiImpl::shutdown() noexcept {
        if(!m_context) return;
        auto* previous = m_state == GuiState::BuildingFrame ? m_previous : ImGui::GetCurrentContext();
        if(previous == m_context) previous = nullptr;
        ImGui::SetCurrentContext(m_context);
        if(m_state == GuiState::BuildingFrame) ImGui::EndFrame();
        m_render.shutdown();
        ImGui::GetIO().Fonts -> SetTexID(nullptr);
        if(m_window && m_cursors[ImGuiMouseCursor_Arrow]) {
            m_window -> setCursor(m_defaultCursor); m_window -> setMouseCursorVisible(true);
        }
        for(auto& cursor : m_cursors) cursor.reset();
        ImGui::DestroyContext(m_context);
        m_context = nullptr;
        m_fonts.clear();
        m_window = nullptr;
        m_previous = nullptr;
        m_state = GuiState::Uninitialized;
        ImGui::SetCurrentContext(previous);
    }

    void GuiImpl::handleEvents(const Event& event) {
        if(m_state != GuiState::Ready) {
            fail("handleEvents requires Ready"); return;
        }
        GuiContextScope scope(m_context);
        feedGuiEvent(ImGui::GetIO(), event);
        if(event.getIf<Event::LostFocus>()) m_windowHasFocus = false;
        else if(event.getIf<Event::GainFocus>()) m_windowHasFocus = true;
    }

    GuiInputCapture GuiImpl::inputCapture() const {
        if(!m_context) return {};
        GuiContextScope scope(m_context);
        return captureGuiInput();
    }

    void GuiImpl::updateMouseCursor() {
        auto& io = ImGui::GetIO();
        if(io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) return;
        const auto cursor = ImGui::GetMouseCursor();
        const bool visible = !io.MouseDrawCursor && cursor != ImGuiMouseCursor_None;
        m_window -> setMouseCursorVisible(visible);
        if(visible) {
            const int index = cursor >= 0 && cursor < ImGuiMouseCursor_COUNT && m_cursors[cursor] -> getRaw()
                ? cursor : ImGuiMouseCursor_Arrow;
            m_window -> setCursor(*m_cursors[index]);
        }
    }

    bool GuiImpl::beginFrame(float dt) {
        if(m_state != GuiState::Ready) return fail("beginFrame requires Ready");
        if(!std::isfinite(dt) || dt <= 0) return fail("beginFrame requires finite positive deltaSeconds");
        m_error.clear();
        m_previous = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(m_context);
        auto& io = ImGui::GetIO();
        const auto logical = m_window -> getLogicalSize();
        const auto pixels = m_window -> getFramebufferSize();
        io.DisplaySize = ImVec2(static_cast<float>(logical.x), static_cast<float>(logical.y));
        io.DisplayFramebufferScale = ImVec2(logical.x ? static_cast<float>(pixels.x) / logical.x : 1.f,
                logical.y ? static_cast<float>(pixels.y) / logical.y : 1.f);
        io.DeltaTime = dt;
        updateMouseCursor();
        if(m_windowHasFocus && io.WantSetMousePos) m_window -> setMousePos({io.MousePos.x, io.MousePos.y});
        ImGui::NewFrame();
        m_state = GuiState::BuildingFrame;
        return true;
    }

    bool GuiImpl::endFrame() {
        if(m_state != GuiState::BuildingFrame) return fail("endFrame requires BuildingFrame");
        ImGui::SetCurrentContext(m_context);
        ImGui::Render();
        m_state = GuiState::DrawDataReady;
        ImGui::SetCurrentContext(m_previous); m_previous = nullptr;
        return true;
    }

    bool GuiImpl::cancelFrame() {
        if(m_state != GuiState::BuildingFrame) return fail("cancelFrame requires BuildingFrame");
        ImGui::SetCurrentContext(m_context);
        ImGui::EndFrame();
        m_state = GuiState::Ready;
        ImGui::SetCurrentContext(m_previous); m_previous = nullptr;
        return true;
    }

    bool GuiImpl::render() {
        if(m_state != GuiState::DrawDataReady) return fail("render requires DrawDataReady");
        // Consume even when skipped/failed: draw data never escapes into the next frame.
        m_state = GuiState::Ready;
        GuiContextScope scope(m_context);
        const auto pixels = m_window -> getFramebufferSize();
        if(pixels.x == 0 || pixels.y == 0) return true;
        try {
            const auto* data = ImGui::GetDrawData();
            if(!data || !m_render.render(*data)) return fail(m_render.lastError());
            return true;
        }
        catch(const std::exception& error) { return fail(std::string("render callback failed: ") + error.what()); }
    }
}
