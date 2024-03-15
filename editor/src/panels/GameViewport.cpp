/*********************************************************************
(c) Alex Raag 2023
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

#include <editor/panels/GameViewport.hpp>
#include <editor/Macro.hpp>
#include <robot2D/imgui/Api.hpp>

namespace editor {

    GameViewport::GameViewport(robot2D::MessageBus& messageBus):
    IPanel(UniqueType(typeid(GameViewport))),
    m_messageBus{messageBus}
    {}

    void GameViewport::render() {
        const auto& colors = ::ImGui::GetStyle().Colors;
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];

        robot2D::WindowOptions windowOptions {
                {
                        {ImGuiStyleVar_WindowPadding, {0, 0}},
                   //     {ImGuiStyleVar_ItemInnerSpacing, {}}
                },
                {
                //        {ImGuiCol_Button, robot2D::Color::fromGL(0.F, 0.F, 0.F, 0.F)},
                 //       {ImGuiCol_ButtonHovered, robot2D::Color::fromGL(buttonHovered.x, buttonHovered.z, 0.5F)},
                  //      {ImGuiCol_ButtonActive,robot2D::Color::fromGL(buttonActive.x, buttonActive.z, 0.5F)}
                }
        };
        windowOptions.name = "Game";
        windowOptions.flagsMask = ImGuiWindowFlags_NoScrollbar;

        robot2D::createWindow(windowOptions, BIND_CLASS_FN(windowFunction));
    }

    void GameViewport::windowFunction() {
        auto ViewPanelSize = ImGui::GetContentRegionAvail();
        if(m_ViewportSize != robot2D::vec2u { static_cast<unsigned int>(ViewPanelSize.x), static_cast<unsigned int>(ViewPanelSize.y)}) {
            m_ViewportSize = { static_cast<unsigned int>(ViewPanelSize.x), static_cast<unsigned int>(ViewPanelSize.y)};
            m_frameBuffer -> Resize(m_ViewportSize);
        }
        robot2D::RenderFrameBuffer(m_frameBuffer, m_ViewportSize.as<float>());
    }

} // namespace editor