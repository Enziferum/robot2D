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

#include <robot2D/imgui/Util.hpp>
#include "GuiImpl.hpp"
#include <robot2D/imgui/InputEvents.hpp>

namespace robot2D::priv {


    GuiImpl::GuiImpl() :
        m_window(nullptr),

        m_windowHasFocus{true},
        m_render{} {

    }

    GuiImpl::~GuiImpl() {
        shutdown();
    }

    void GuiImpl::setup(robot2D::Window& window, bool createFonts) {
        m_window = &window;
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
        io.BackendPlatformName = "ImGui_robot2D";
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        /*
            io.ClipboardUserData = (GLFWwindow*)m_window->getRaw();
            io.GetClipboardTextFn = Glfw_GetClipboardText;
            io.SetClipboardTextFn = Glfw_SetClipboardText;
         */

        auto sz = window.getSize();
        io.DisplaySize = ImVec2(static_cast<float>(sz.x),
                static_cast<float>(sz.y));

        m_cursors[ImGuiMouseCursor_Arrow].createDefault();
        m_cursors[ImGuiMouseCursor_TextInput].create(robot2D::CursorType::TextInput);
        m_cursors[ImGuiMouseCursor_ResizeNS].create(robot2D::CursorType::ResizeUpDown);
        m_cursors[ImGuiMouseCursor_ResizeEW].create(robot2D::CursorType::ResizeLeftRight);
        m_cursors[ImGuiMouseCursor_Hand].create(robot2D::CursorType::Hand);
        m_cursors[ImGuiMouseCursor_ResizeAll].createDefault();
        m_cursors[ImGuiMouseCursor_ResizeNESW].createDefault();
        m_cursors[ImGuiMouseCursor_ResizeNWSE].createDefault();
        m_cursors[ImGuiMouseCursor_NotAllowed].createDefault();

        m_render.setup(createFonts);
    }


    void GuiImpl::handleEvents(const robot2D::Event& event) {
        feedGuiEvent(ImGui::GetIO(), event);
        if(event.getIf<Event::LostFocus>()) m_windowHasFocus = false;
        else if(event.getIf<Event::GainFocus>()) m_windowHasFocus = true;
    }

    void GuiImpl::updateMouseCursor() {
        ImGuiIO& io = ImGui::GetIO();
        if((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0) {
            ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
            if(io.MouseDrawCursor || cursor == ImGuiMouseCursor_None) {
                m_window -> setMouseCursorVisible(false);
            }
            else {
                m_window -> setMouseCursorVisible(true);
                if(cursor < ImGuiMouseCursor_COUNT)
                    m_window -> setCursor(m_cursors[cursor]);
                else
                    m_window -> setCursor(m_cursors[ImGuiMouseCursor_Arrow]);
            }
        }
    }

    void GuiImpl::update(float dt) {
        ImGuiIO& io = ImGui::GetIO();
        updateMouseCursor();

        auto sz = m_window -> getSize();
        io.DisplaySize = ImVec2(static_cast<float>(sz.x),
                static_cast<float>(sz.y));
        io.DeltaTime = dt;
        if(m_windowHasFocus) {
            if(io.WantSetMousePos) {
                robot2D::vec2f newMousePos = {io.MousePos.x, io.MousePos.y};
                m_window -> setMousePos(newMousePos);
            }
        }

        if(io.MouseDrawCursor)
            m_window -> setMouseCursorVisible(false);

        ImGui::NewFrame();
    }

    void GuiImpl::render() {
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        if(draw_data)
            m_render.render(draw_data);
    }

    void GuiImpl::shutdown() {
        ImGui::GetIO().Fonts -> SetTexID(ImGui::convertTextureHandle(0));
        ImGui::DestroyContext();
    }

    void GuiImpl::setupFonts(std::vector<GuiFontConfig>&& fontConfigs) {
        m_render.setupFonts(std::move(fontConfigs));
    }

} // namespace robot2D