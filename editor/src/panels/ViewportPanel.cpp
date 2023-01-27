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

#include <editor/panels/ViewportPanel.hpp>
#include <editor/Messages.hpp>
#include <editor/Components.hpp>

#include "ImGuizmo.h"

#ifdef USE_GLM
#define GLM_ENABLE_EXPERIMENTAL
    #include <glm/gtx/matrix_decompose.hpp>
    #include "glm/gtc/type_ptr.hpp"
#endif

namespace editor {
#ifdef USE_GLM
    glm::mat4 toGLM(const robot2D::mat4& matrix) {
        auto m = matrix;
        return {
                m[0],m[1],m[2],m[3],
                m[4],m[5],m[6],m[7],
                m[8],m[9],m[10],m[11],
                m[12],m[13],m[14],m[15]
        };
    }
#endif

    bool DecomposeTransform(const robot2D::mat4& transform, robot2D::vec3f& translation,
                            robot2D::vec3f& rotation, robot2D::vec3f& scale)
    {
#ifdef USE_GLM
        using T = float;

        robot2D::mat4 LocalMatrix(transform);

        // Normalize the matrix.
        if (robot2D::epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), robot2D::epsilon<T>()))
            return false;

        // First, isolate perspective.  This is the messiest.
        if (
                robot2D::epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), robot2D::epsilon<T>()) ||
                        robot2D::epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), robot2D::epsilon<T>()) ||
                glm::epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), glm::epsilon<T>()))
        {
            // Clear the perspective partition
            LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
            LocalMatrix[3][3] = static_cast<T>(1);
        }

        // Next take care of translation (easy).
        auto transl = glm::vec3(LocalMatrix[3]);
        LocalMatrix[3] = glm::vec4(0, 0, 0, LocalMatrix[3].w);

        glm::vec3 Row[3], Pdum3;

        // Now get scale and shear.
        for (glm::length_t i = 0; i < 3; ++i)
            for (glm::length_t j = 0; j < 3; ++j)
                Row[i][j] = LocalMatrix[i][j];

        // Compute X scale factor and normalize first row.
        scale.x = length(Row[0]);
        Row[0] = glm::detail::scale(Row[0], static_cast<T>(1));
        scale.y = length(Row[1]);
        Row[1] = glm::detail::scale(Row[1], static_cast<T>(1));
        scale.z = length(Row[2]);
        Row[2] = glm::detail::scale(Row[2], static_cast<T>(1));


        rotation.y = asin(-Row[0][2]);
        if (cos(rotation.y) != 0) {
            rotation.x = atan2(Row[1][2], Row[2][2]);
            rotation.z = atan2(Row[0][1], Row[0][0]);
        }
        else {
            rotation.x = atan2(-Row[2][0], Row[1][1]);
            rotation.z = 0;
        }

        translation = {transl.x, transl.y, transl.z};
#endif
        return true;
    }



    ViewportPanel::ViewportPanel(
            IUIManager& uiManager,
            EditorCamera& editorCamera,
            robot2D::MessageBus& messageBus,
            Scene::Ptr&& scene):
        IPanel(UniqueType(typeid(ViewportPanel))),
        m_uiManager{uiManager},
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
        m_windowOptions.flagsMask = ImGuiWindowFlags_NoScrollbar;
        m_windowOptions.name = "##Viewport";
    }


    void ViewportPanel::update(float deltaTime) {
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

        if(m_panelFocused && m_panelHovered)
            m_editorCamera.update({mx, my}, deltaTime);
    }

    void ViewportPanel::render() {
        ImGui::createWindow(m_windowOptions, [this]() {
            auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
            auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
            auto viewportOffset = ImGui::GetWindowPos();
            m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
            m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

            m_panelFocused = ImGui::IsWindowFocused();
            m_panelHovered = ImGui::IsWindowHovered();

            /// TODO: @a.raag switch mode of camera ///
            auto ViewPanelSize = ImGui::GetContentRegionAvail();

            if(m_ViewportSize != robot2D::vec2u { ViewPanelSize.x, ViewPanelSize.y}) {
                m_ViewportSize = {ViewPanelSize.x, ViewPanelSize.y};
                m_frameBuffer -> Resize(m_ViewportSize);
                m_editorCamera.setViewportSize(m_ViewportSize.as<float>());
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

            int m_GizmoType = 0;
            if (selectedEntity && m_GizmoType != -1)
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();

                ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y,
                                  m_ViewportBounds[1].x - m_ViewportBounds[0].x,
                                  m_ViewportBounds[1].y - m_ViewportBounds[0].y);

                // Editor camera
                const robot2D::mat4& cameraProjection = m_editorCamera.getProjectionMatrix();
                robot2D::mat4 cameraView = m_editorCamera.getViewMatrix();

                // Entity transform
                auto& tc = selectedEntity.getComponent<Transform3DComponent>();
                auto transform = tc.getTransform();

                // Snapping
                bool snap = robot2D::Keyboard::isKeyPressed(robot2D::Key::LEFT_ALT);
                float snapValue = 0.5f; // Snap to 0.5m for translation/scale
                // Snap to 45 degrees for rotation
                if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
                    snapValue = 45.0f;

                float snapValues[3] = { snapValue, snapValue, snapValue };
                m_GizmoType = ImGuizmo::TRANSLATE;

#ifdef USE_GLM
                auto glmMatrix = toGLM(transform);

                ImGuizmo::Manipulate(cameraView.getRaw(), cameraProjection.getRaw(),
                                     (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL,
                                     glm::value_ptr(glmMatrix),
                                     nullptr, snap ? snapValues : nullptr);
#endif
                if (ImGuizmo::IsUsing())
                {
                    robot2D::vec3f translation, rotation, scale;
                    DecomposeTransform(transform, translation, rotation, scale);
                    auto delta = rotation - tc.getRotation();
                    tc.getRotation() += delta;
                    tc.setPosition(translation);
                    tc.setScale(scale);
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
