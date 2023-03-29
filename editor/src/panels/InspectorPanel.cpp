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
#include <editor/panels/InspectorPanel.hpp>

namespace editor {

    InspectorPanel::InspectorPanel(IEditorCamera::Ptr sceneCamera):
    IPanel(UniqueType(typeid(InspectorPanel))),
    m_camera{sceneCamera},
    m_configuration{} {
        m_clearColor = m_configuration.defaultBackGround;
    }

    const robot2D::Color& InspectorPanel::getColor() const {
        return m_clearColor;
    }

    void InspectorPanel::setRenderStats(robot2D::RenderStats&& renderStats) {
        m_renderStats = std::move(renderStats);
    }

    void InspectorPanel::render() {
        ImGui::Begin("Statistics ");

        // TODO(a.raag) need apply graphics styles

        ImGui::Text("Rendering 2D Stats ...");

        ImGui::Text("Quads Count: %d", m_renderStats.drawQuads);
        ImGui::Text("Draw Calls Count: %d", m_renderStats.drawCalls);

        if(m_camera -> getType() == EditorCameraType::Orthographic) {
            auto& position = m_camera -> getPosition();
            auto view = m_camera -> getView().getSize();
            // TODO (a.raag): add Size
            ImGui::Text("Viewport Position := %.2f, %.2f", position.x, position.y);
        }
        else if(m_camera -> getType() == EditorCameraType::Perspective) {
            auto cameraView = m_camera -> getViewMatrix();
            auto& position = m_camera -> getPosition();
            ImGui::Text("Viewport Position := %.2f, %.2f, %.2f", position.x, position.y, position.z);
        }


        auto color = m_clearColor.toGL();
        float colors[4];
        colors[0] = color.red; colors[1] = color.green;
        colors[2] = color.blue; colors[3] = color.alpha;
        ImGui::ColorEdit4("Color", colors);
        m_clearColor = {robot2D::Color::fromGL(colors[0], colors[1], colors[2], colors[3])};
        ImGui::End();
    }

}