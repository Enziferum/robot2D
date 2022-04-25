/*********************************************************************
(c) Alex Raag 2021
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

#include <imgui/imgui.h>
#include <editor/ViewportPanel.hpp>
#include <robot2D/Extra/Api.hpp>

#include <filesystem>

namespace editor {

    ViewportPanel::ViewportPanel(Scene::Ptr&& scene):
        IPanel(UniqueType(typeid(ViewportPanel))),
    m_scene(std::move(scene)),
    m_frameBuffer(nullptr) {}

    void ViewportPanel::render() {
        return;
        robot2D::vec2u  m_ViewportSize{};

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        ImGui::Begin("##Viewport", nullptr, ImGuiWindowFlags_NoTitleBar);

        auto ViewPanelSize = ImGui::GetContentRegionAvail();

        if(m_ViewportSize != robot2D::vec2u { ViewPanelSize.x, ViewPanelSize.y}) {
            m_ViewportSize = {ViewPanelSize.x, ViewPanelSize.y};
            m_frameBuffer -> Resize(m_ViewportSize);
        }

        ImGui::RenderFrameBuffer(m_frameBuffer, {m_ViewportSize.x, m_ViewportSize.y});
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
#ifdef ROBOT2D_WINDOWS
                const wchar_t* path = (const wchar_t*)payload->Data;
#else
                const char* path = (const char*)payload->Data;
#endif
                std::filesystem::path scenePath = std::filesystem::path("assets") / path;
                RB_EDITOR_INFO("PATH {0}", scenePath.string().c_str());
               // openScene(scenePath.string());
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}
