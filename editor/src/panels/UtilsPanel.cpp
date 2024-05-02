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

    UtilPanel::UtilPanel(IEditorCamera::Ptr sceneCamera):
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

    void UtilPanel::render() {
        ImGui::Begin("Statistics ");

        // TODO(a.raag) need apply graphics styles

        ImGui::Text("Rendering 2D Stats ...");

        ImGui::Text("Quads Count: %d", m_renderStats.drawQuads);
        ImGui::Text("Draw Calls Count: %d", m_renderStats.drawCalls);

        if(m_camera -> getType() == EditorCameraType::Orthographic) {
            auto& position = m_camera -> getView().getCenter();
            auto size = m_camera -> getView().getSize();
            ImGui::Text("Viewport center := %.2f, %.2f", position.x, position.y);
            ImGui::Text("Viewport size := %.2f, %.2f", size.x, size.y);
            int i_size[2] = { (int)size.x, (int)size.y };
            ImGui::InputInt2("Viewport size", i_size);
            m_camera -> getView().setSize(i_size[0], i_size[1]);
            ImGui::Text("Zoom := %.2f", m_camera -> getZoom());
        }
        else if(m_camera -> getType() == EditorCameraType::Perspective) {
            auto cameraView = m_camera -> getViewMatrix();
            auto& position = m_camera -> getPosition();
            ImGui::Text("Viewport Position := %.2f, %.2f, %.2f", position.x, position.y, position.z);
        }

        if(ImGui::Button("Reset"))
            m_camera -> resetDefaults();


        auto color = m_clearColor.toGL();
        float colors[4];
        colors[0] = color.red; colors[1] = color.green;
        colors[2] = color.blue; colors[3] = color.alpha;
        ImGui::ColorEdit4("Color", colors);
        m_clearColor = {robot2D::Color::fromGL(colors[0], colors[1], colors[2], colors[3])};
        ImGui::End();
    }

}