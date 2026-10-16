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

#include <robot2D/imgui/Gui.hpp>
#include "GuiImpl.hpp"

namespace robot2D {
    Gui::Gui() : m_impl(std::make_unique<priv::GuiImpl>()) {
    }
    Gui::~Gui() noexcept = default;
    bool Gui::initialize(const GuiConfiguration& configuration) {
        return m_impl -> initialize(configuration);
    }
    void Gui::shutdown() noexcept {
        m_impl -> shutdown();
    }
    void Gui::handleEvents(const Event& event) {
        m_impl -> handleEvents(event);
    }
    GuiInputCapture Gui::inputCapture() const {
        return m_impl -> inputCapture();
    }
    bool Gui::beginFrame(float dt) {
        return m_impl -> beginFrame(dt);
    }
    bool Gui::endFrame() {
        return m_impl -> endFrame();
    }
    bool Gui::cancelFrame() {
        return m_impl -> cancelFrame();
    }
    bool Gui::render() {
        return m_impl -> render();
    }
    GuiState Gui::state() const noexcept {
        return m_impl -> state();
    }
    const std::string& Gui::lastError() const noexcept {
        return m_impl -> lastError();
    }
    ImGuiContext* Gui::context() const noexcept {
        return m_impl -> context();
    }
    ImFont* Gui::font(FontId id) const noexcept {
        return m_impl -> font(id);
    }
}
