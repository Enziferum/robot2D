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

#include <robot2D/imgui/Api.hpp>
#include <editor/panels/UtilPanel.hpp>

namespace editor {
    #define  ICON u8"\ue000"

    UtilPanel::UtilPanel(IEditorCamera::Ptr sceneCamera) :
        IPanel(UniqueType(typeid(UtilPanel))),
        m_camera{sceneCamera},
        m_configuration{} {
        m_clearColor = m_configuration.defaultBackGround;
    }

    const robot2D::Color& UtilPanel::getColor() const {
        return m_clearColor;
    }

    void UtilPanel::setRenderStats(robot2D::RenderStats&& renderStats) {
        m_renderStats = std::move(renderStats);
    }

    float UtilPanel::getHeight() {
        return ImGui::GetFrameHeight() + 8.f;
    }

    void UtilPanel::render() {
        auto* viewport = ImGui::GetMainViewport();
        auto height = getHeight();
        ImGui::SetNextWindowViewport(viewport -> ID);
        ImGui::SetNextWindowPos({viewport -> WorkPos.x, viewport -> WorkPos.y + viewport -> WorkSize.y - height});
        ImGui::SetNextWindowSize({viewport -> WorkSize.x, height});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.f, 4.f});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        auto flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        ImGui::Begin("##EditorStatusBar", nullptr, flags);
        auto fps = ImGui::GetIO().Framerate;
        ImGui::AlignTextToFramePadding();
        if(viewport -> WorkSize.x >= 900.f)
            ImGui::TextDisabled("%.1f ms  |  %.0f FPS  |  Quads: %u  |  Draw calls: %u",
                fps > 0.f ? 1000.f / fps : 0.f, fps, m_renderStats.drawQuads, m_renderStats.drawCalls);
        else
            ImGui::TextDisabled("Quads: %u  |  Draw calls: %u", m_renderStats.drawQuads, m_renderStats.drawCalls);
        if(m_camera && viewport -> WorkSize.x >= 900.f) {
            ImGui::SameLine(0.f, 16.f);
            ImGui::TextDisabled("Zoom: %.2f", m_camera -> getZoom());
        }
        ImGui::SameLine(0.f, 16.f);
        if(ImGui::Button("Viewport"))
            ImGui::OpenPopup("ViewportSettings");
        if(ImGui::BeginPopup("ViewportSettings")) {
            if(m_camera) {
                if(m_camera -> getType() == EditorCameraType::Orthographic) {
                    auto position = m_camera -> getView().getCenter();
                    auto size = m_camera -> getView().getSize();
                    ImGui::Text("Center: %.2f, %.2f", position.x, position.y);
                    ImGui::Text("Visible size: %.2f, %.2f", size.x, size.y);
                    float percent = m_camera -> getZoom() * 100.f;
                    if(ImGui::InputFloat("Zoom (%)", &percent, 25.f, 100.f, "%.0f"))
                        m_camera -> setZoom(percent / 100.f);
                }
                else {
                    auto position = m_camera -> getPosition();
                    ImGui::Text("Position: %.2f, %.2f, %.2f", position.x, position.y, position.z);
                }
                if(ImGui::Button("Reset camera"))
                    m_camera -> resetDefaults();
            }
            auto color = m_clearColor.toGL();
            float values[4] = {color.red, color.green, color.blue, color.alpha};
            if(ImGui::ColorEdit4("Background", values))
                m_clearColor = robot2D::Color::fromGL(values[0], values[1], values[2], values[3]);
            ImGui::EndPopup();
        }
        ImGui::End();
        ImGui::PopStyleVar(2);
    }
}
