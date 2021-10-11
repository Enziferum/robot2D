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

#pragma once

#include <imgui/imgui.h>
#include <editor/InspectorPanel.hpp>

namespace editor {
    const robot2D::Color defaultBackGround = robot2D::Color::fromGL(0.1, 0.1, 0.1, 1);

    InspectorPanel::InspectorPanel(SceneCamera& sceneCamera): IPanel(UniqueType(typeid(InspectorPanel))),
    m_camera{sceneCamera}{
        m_clearColor = defaultBackGround;
    }

    const robot2D::Color& InspectorPanel::getColor() const {
        return m_clearColor;
    }

    void InspectorPanel::setRenderStats(robot2D::RenderStats&& renderStats) {
        m_renderStats = std::move(renderStats);
    }

    void InspectorPanel::render() {
        ImGui::Begin("Statistics ");

        // todo Apply some styles

        ImGui::Text("Rendering 2D Stats ...");

        ImGui::Text("Quads Count: %d", m_renderStats.drawQuads);
        ImGui::Text("Draw Calls Count: %d", m_renderStats.drawCalls);
        // todo zoom precision
        ImGui::Text("Camera Zoom: %.2f", m_camera.getZoom());
        ImGui::Text("Camera Speed: ");
        ImGui::SameLine();
        ImGui::DragFloat("##Y", &m_camera.getCameraSpeed(), 0.1f, 10.0f, 100.0f, "%.2f");

        auto cameraCenter = m_camera.getView().getCenter();
        auto cameraSize = m_camera.getView().getSize();
        ImGui::Text("Viewport Center := %.2f, %.2f", cameraCenter.x, cameraCenter.y);
        ImGui::Text("Viewport Size := %.2f, %.2f", cameraSize.x, cameraSize.y);

        auto color = m_clearColor.toGL();
        float colors[4];
        colors[0] = color.red; colors[1] = color.green;
        colors[2] = color.blue; colors[3] = color.alpha;
        ImGui::ColorEdit4("Color", colors);
        m_clearColor = {robot2D::Color::fromGL(colors[0], colors[1], colors[2], colors[3])};
        ImGui::End();
    }

}