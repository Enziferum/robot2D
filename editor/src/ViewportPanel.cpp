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


#include <filesystem>

#include <imgui/imgui.h>
#include <robot2D/Extra/Api.hpp>

#include <editor/ViewportPanel.hpp>
#include <editor/Messages.hpp>
#include "ImGuizmo.h"
#include <editor/Components.hpp>

namespace editor {

    ViewportPanel::ViewportPanel(IUIManager& uiManager, SceneCamera& sceneCamera, EditorCamera& editorCamera,
                                 robot2D::MessageBus& messageBus,  Scene::Ptr&& scene):
        IPanel(UniqueType(typeid(ViewportPanel))),
        m_uiManager{uiManager},
        m_sceneCamera{sceneCamera},
        m_editorCamera{editorCamera},
        m_messageBus{messageBus},
        m_scene(std::move(scene)),
        m_frameBuffer(nullptr) ,
        m_panelFocused{false},
        m_panelHovered{false}
        {
        m_windowOptions = ImGui::WindowOptions {
                {
                        {ImGuiStyleVar_WindowPadding, {0, 0}}
                },
                {}
        };
        m_windowOptions.flagsMask = ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoScrollbar;
        m_windowOptions.name = "##Viewport";
    }


    void ViewportPanel::update() {
        auto[mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        robot2D::vec2f viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
        my = viewportSize.y - my;
        int mouseX = (int)mx;
        int mouseY = (int)my;

        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
        {
        }
    }


    bool DecomposeTransform(const robot2D::Transform& transform, robot2D::vec3f& translation,
                            robot2D::vec3f& rotation, robot2D::vec3f& scale)
    {

    }

    robot2D::Transform tr;
    void ViewportPanel::render() {
        ImGui::createWindow(m_windowOptions, [this]() {
            auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
            auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
            auto viewportOffset = ImGui::GetWindowPos();
            m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
            m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

            m_panelFocused = ImGui::IsWindowFocused();
            m_panelHovered = ImGui::IsWindowHovered();

            //m_uiManager.blockEvents(!m_panelHovered && !m_panelFocused);

            /// TODO: @a.raag switch mode of camera ///
            auto ViewPanelSize = ImGui::GetContentRegionAvail();

            if(m_ViewportSize != robot2D::vec2u { ViewPanelSize.x, ViewPanelSize.y}) {
                m_ViewportSize = {ViewPanelSize.x, ViewPanelSize.y};
                m_frameBuffer -> Resize(m_ViewportSize);
            }

            auto[mx, my] = ImGui::GetMousePos();
            mx -= m_ViewportBounds[0].x;
            my -= m_ViewportBounds[0].y;
            robot2D::vec2f viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
            my = viewportSize.y - my;
            int mouseX = (int)mx;
            int mouseY = (int)my;

            if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
            {
                if(ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    int pixelData = m_frameBuffer -> readPixel(1, {mouseX, mouseY});
                    auto msg = m_messageBus.postMessage<EntitySelection>(EntitySelected);
                    msg -> entityID = pixelData;
                }
            }

            ImGui::RenderFrameBuffer(m_frameBuffer, m_ViewportSize.as<float>());
            robot2D::ecs::Entity selectedEntity = m_uiManager.getSelectedEntity();
            //selectedEntity && m_GizmoType != -1

            int m_GizmoType = 0;
            if (selectedEntity)
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();

                ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y,
                                  m_ViewportBounds[1].x - m_ViewportBounds[0].x,
                                  m_ViewportBounds[1].y - m_ViewportBounds[0].y);

                // Camera

                // Runtime camera from entity
                // auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
                // const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
                // const glm::mat4& cameraProjection = camera.GetProjection();
                // glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

                // Editor camera
                const mat4& cameraProjection = m_editorCamera.getProjectionMatrix();
                mat4 cameraView = m_editorCamera.getViewMatrix();

                // Entity transform
                auto& tc = selectedEntity.getComponent<TransformComponent>();
                auto transform = tc.getTransform();

                // Snapping
                bool snap = robot2D::Keyboard::isKeyPressed(robot2D::Key::LEFT_ALT);
                float snapValue = 0.5f; // Snap to 0.5m for translation/scale
                // Snap to 45 degrees for rotation
                if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
                    snapValue = 45.0f;

                float snapValues[3] = { snapValue, snapValue, snapValue };
                m_GizmoType = ImGuizmo::TRANSLATE;

                float* mm = const_cast<float *>(transform.get_matrix());

                ImGuizmo::Manipulate(cameraView.getRaw(), cameraProjection.getRaw(),
                                     (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL,
                                     mm,
                                     nullptr, snap ? snapValues : nullptr);

                if (ImGuizmo::IsUsing())
                {
                    robot2D::vec3f translation, rotation, scale;
                }
            }

            ImGui::dummyDragDrop("CONTENT_BROWSER_ITEM", [](std::string path){
                const wchar_t* rawPath = (const wchar_t*)path.c_str();
                std::filesystem::path scenePath = std::filesystem::path("assets") / rawPath;
                /// TODO: @a.raag send MessageQueue to open Scene
            });
        });
    }
}
