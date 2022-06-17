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
        m_frameBuffer(nullptr) {
        m_windowOptions = ImGui::WindowOptions {
                {
                        {ImGuiStyleVar_WindowPadding, {0, 0}}
                },
                {}
        };
        m_windowOptions.flagsMask = ImGuiWindowFlags_NoTitleBar;
        m_windowOptions.name = "##Viewport";
    }

    void ViewportPanel::render() {
        ImGui::createWindow(m_windowOptions, [this]() {
            auto ViewPanelSize = ImGui::GetContentRegionAvail();

            if(m_ViewportSize != robot2D::vec2u { ViewPanelSize.x, ViewPanelSize.y}) {
                m_ViewportSize = {ViewPanelSize.x, ViewPanelSize.y};
                m_frameBuffer -> Resize(m_ViewportSize);
            }

            ImGui::RenderFrameBuffer(m_frameBuffer, m_ViewportSize.as<float>());

            ImGui::dummyDragDrop("CONTENT_BROWSER_ITEM", [](std::string path){
                // todo validate input path
                const wchar_t* rawPath = (const wchar_t*)path.c_str();
                std::filesystem::path scenePath = std::filesystem::path("assets") / rawPath;
                RB_EDITOR_INFO("PATH Got {0}", scenePath.string());
            });
        });
    }
}
