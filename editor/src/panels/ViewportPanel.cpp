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


#include <filesystem>
#include <algorithm>
#include <cmath>
#include <cstdio>

#include <robot2D/imgui/Api.hpp>
#include <robot2D/imgui/DragDropTarget.hpp>
#include <imgui/imgui.h>

#include <editor/panels/ViewportPanel.hpp>
#include <editor/Messages.hpp>
#include <editor/Components.hpp>
#include <editor/DragDropIDS.hpp>
#include <editor/Buffer.hpp>

#include "../IconsFontsAwesome5.hpp"

namespace editor {
    ViewportPanel::ViewportPanel(
        UIInteractor* uiInteractor,
        IEditorCamera::Ptr editorCamera,
        robot2D::MessageBus& messageBus,
        MessageDispatcher& messageDispatcher,
        Guizmo2D& guizmo2D,
        CameraManipulator& collider,
        SelectionCollider& selectionCollider) :
        IPanel(UniqueType(typeid(ViewportPanel))),
        m_uiInteractor{uiInteractor},
        m_editorCamera{editorCamera},
        m_messageBus{messageBus},
        m_messageDispatcher{messageDispatcher},
        m_selectionCollider{selectionCollider},
        m_guizmo2D{guizmo2D},
        m_CameraCollider{collider} {
        m_windowOptions = robot2D::WindowOptions {
            {
                {ImGuiStyleVar_WindowPadding, {0, 0}}
            },
            {}
        };
        m_windowOptions.flagsMask = ImGuiWindowFlags_NoScrollbar;
        m_windowOptions.name = ICON_FA_ETHERNET + std::string{" Viewport"} + "###ViewportPanel";


    }

    void ViewportPanel::setFramebuffer(robot2D::FrameBuffer::Ptr frameBuffer) {
        m_frameBuffer = std::move(frameBuffer);
        needResetViewport = true;
    }


    bool ViewportPanel::containsMouse(robot2D::vec2f position) const {
        return getState() == PanelState::Visible && m_panelHovered
               && position.x >= m_ViewportBounds.minRegion.x && position.y >= m_ViewportBounds.minRegion.y
               && position.x < m_ViewportBounds.maxRegion.x && position.y < m_ViewportBounds.maxRegion.y;
    }

    void ViewportPanel::handleEvents(const robot2D::Event& event) {
        if(getState() != PanelState::Visible)
            return;
        if(event.type == robot2D::Event::MousePressed && m_panelFocused && m_panelHovered) {
            auto[mx, my] = ImGui::GetMousePos();
            if(mx < m_ViewportBounds.minRegion.x || my < m_ViewportBounds.minRegion.y
                || mx >= m_ViewportBounds.maxRegion.x || my >= m_ViewportBounds.maxRegion.y)
                return;
            mx -= m_ViewportBounds.minRegion.x;
            my -= m_ViewportBounds.minRegion.y;
            robot2D::vec2f viewportSize = m_ViewportBounds.maxRegion - m_ViewportBounds.minRegion;
            my = viewportSize.y - my;

            if(!m_guizmo2D.isActive() && !m_CameraCollider.isActive() ) {
                robot2D::vec2i mousePos = { static_cast<int>(mx), static_cast<int>(my) };
                m_uiInteractor -> findEntity(mousePos);
            }
        }
    }

    void ViewportPanel::update(float deltaTime) {
        if(getState() != PanelState::Visible) {
            m_guizmo2D.setIsShow(false);
            m_guizmo2D.setManipulated(nullptr);
            return;
        }
        auto[mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds.minRegion.x;
        my -= m_ViewportBounds.minRegion.y;

        robot2D::vec2f viewportSize = m_ViewportBounds.maxRegion - m_ViewportBounds.minRegion;
        my = viewportSize.y - my;

        if(m_panelFocused && m_panelHovered) {
            if(mx >= 0.f && my >= 0.f && mx < viewportSize.x && my < viewportSize.y)
                m_editorCamera -> update({mx, my}, deltaTime);
        }
    }

    void ViewportPanel::render() {
        m_panelFocused = false;
        m_panelHovered = false;
        bool hasImage = false;
        renderWindow(m_windowOptions, [this, &hasImage]() {
            instrumentBar();
            auto imagePosition = ImGui::GetCursorScreenPos();
            auto ViewPanelSize = ImGui::GetContentRegionAvail();
            if(!m_frameBuffer || !m_editorCamera || !m_uiInteractor
            || ViewPanelSize.x < 1.f || ViewPanelSize.y < 1.f)
                return;

            m_ViewportBounds.minRegion = {imagePosition.x, imagePosition.y};
            m_editorCamera -> setViewportBounds(m_ViewportBounds.minRegion);
            robot2D::vec2u viewportSize{static_cast<unsigned int>(ViewPanelSize.x),
                                        static_cast<unsigned int>(ViewPanelSize.y)};
            if(m_ViewportSize != viewportSize || needResetViewport) {
                m_ViewportSize = viewportSize;
                m_frameBuffer -> Resize(m_ViewportSize);
                m_editorCamera -> setViewportSize(m_ViewportSize.as<float>());
                needResetViewport = false;
            }
            m_ViewportBounds.maxRegion = m_ViewportBounds.minRegion + m_ViewportSize.as<float>();
            robot2D::RenderFrameBuffer(m_frameBuffer, m_ViewportSize.as<float>());
            hasImage = true;
            m_panelFocused = ImGui::IsWindowFocused();
            m_panelHovered = ImGui::IsItemHovered();
            m_CameraCollider.setIsShownDots(false);

            auto selectedEntities = m_uiInteractor -> getSelectedEntities();

            if(selectedEntities.size() == 1) {
                auto& selectedEntity = selectedEntities[0];
                if(selectedEntity) {
                    // Editor camera
                    const robot2D::mat4& cameraProjection = m_editorCamera -> getProjectionMatrix();
                    robot2D::mat4 cameraView = m_editorCamera -> getViewMatrix();

                    auto& tc = selectedEntity.getComponent<TransformComponent>();
                    m_guizmo2D.setIsShow(true);
                    m_guizmo2D.setManipulated(&tc);

                    if(selectedEntity.hasComponent<CameraComponent>()) {
                        m_CameraCollider.setIsShownDots(true);
                        auto& transform = selectedEntity.getComponent<TransformComponent>();
                        auto position = transform.getPosition();
                        auto frame = m_CameraCollider.getRect();

                        robot2D::vec2f colliderPosition = {
                            position.x - frame.width / 2.f,
                            position.y - frame.height / 2.f,
                        };

                        // m_CameraCollider.setSize(m_selectedEntity.getComponent<CameraComponent>().orthoSize);
                        selectedEntity.getComponent<CameraComponent>().size = {frame.width, frame.height};
                        selectedEntity.getComponent<CameraComponent>().position = frame.topPoint();
                        selectedEntity.getComponent<CameraComponent>().orthoSize = m_CameraCollider.getSize();
                        selectedEntity.getComponent<CameraComponent>().cameraRect = m_CameraCollider.getCameraRect();
                    }
                }
                else {
                    m_guizmo2D.setIsShow(false);
                    m_guizmo2D.setManipulated(nullptr);
                }
            }
            else if(selectedEntities.size() > 1) {
                std::vector<TransformComponent*> moveTfs;
                for(auto& entity: selectedEntities) {
                    if(entity) {
                        auto& tc = entity.getComponent<TransformComponent>();
                        moveTfs.emplace_back(&tc);
                    }
                }
                m_guizmo2D.setIsShow(!moveTfs.empty());
                m_guizmo2D.setManipulated(moveTfs);
                std::vector<TransformComponent*> movementRoots;
                for(auto entity: selectedEntities) {
                    if(!entity)
                        continue;
                    bool selectedAncestor = false;
                    for(auto parent = entity.getParent(); parent; parent = parent.getParent()) {
                        if(std::find(selectedEntities.begin(), selectedEntities.end(),
                        parent) != selectedEntities.end()) {
                            selectedAncestor = true;
                            break;
                        }
                    }
                    if(!selectedAncestor)
                        movementRoots.push_back(&entity.getComponent<TransformComponent>());
                }
                m_guizmo2D.setMovementRoots(movementRoots);
            }
            else {
                m_guizmo2D.setIsShow(false);
                m_guizmo2D.setManipulated(nullptr);
            }

            const auto mousePosition = ImGui::GetMousePos();
            m_guizmo2D.updateHover({mousePosition.x, mousePosition.y}, m_panelHovered);
            {
                robot2D::DragDropTarget dragDropTarget{};
                if(auto && payloadBuffer = dragDropTarget.unpackPayload2Buffer(contentSceneID,
                ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
                    auto&& path = payloadBuffer.unpack<std::string>();
                    std::filesystem::path scenePath = std::filesystem::path("assets") / path;

                    auto messagePath = scenePath.string();
                    int allocSize = StringBuffer::calcAllocSize(messagePath);
                    void* rawBuffer = m_messageBus.postMessage(MessageID::OpenScene, allocSize);
                    Buffer buffer { rawBuffer };
                    pack_message_string(messagePath, buffer);
                }

                if(auto && payloadBuffer = dragDropTarget.unpackPayload2Buffer(contentPrefabItemID,
                ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
                    auto&& path = payloadBuffer.unpack<std::string>();
                    std::filesystem::path prefabPath = std::filesystem::path("assets") / path;

                    auto messagePath = prefabPath.string();
                    int allocSize = StringBuffer::calcAllocSize(messagePath);
                    void* rawBuffer = m_messageBus.postMessage(MessageID::PrefabLoad, allocSize);
                    Buffer buffer { rawBuffer };
                    pack_message_string(messagePath, buffer);
                }
            }
        });
        if(!hasImage) {
            m_guizmo2D.setIsShow(false);
            m_guizmo2D.setManipulated(nullptr);
        }
    }


    bool ViewportPanel::frameEntities(bool wholeScene) {
        if(!m_uiInteractor || !m_editorCamera || m_guizmo2D.isActive())
            return false;
        std::vector<SceneEntity> entities;
        if(wholeScene) {
            const auto& sceneEntities = m_uiInteractor -> getEntities();
            entities.assign(sceneEntities.begin(), sceneEntities.end());
        }
        else
            entities = m_uiInteractor -> getSelectedEntities();
        bool found = false;
        robot2D::FloatRect bounds;
        for(const auto& entity: entities) {
            if(!entity || !entity.hasComponent<TransformComponent>())
                continue;
            const auto rect = entity.getComponent<TransformComponent>().getGlobalBounds();
            if(!std::isfinite(rect.lx) || !std::isfinite(rect.ly) || !std::isfinite(rect.width)
                || !std::isfinite(rect.height) || rect.width < 0.f || rect.height < 0.f)
                continue;
            const float right = rect.lx + rect.width;
            const float bottom = rect.ly + rect.height;
            if(!std::isfinite(right) || !std::isfinite(bottom))
                continue;
            if(!found) {
                bounds = rect;
                found = true;
            }
            else {
                const float maxX = std::max(bounds.lx + bounds.width, right);
                const float maxY = std::max(bounds.ly + bounds.height, bottom);
                bounds.lx = std::min(bounds.lx, rect.lx);
                bounds.ly = std::min(bounds.ly, rect.ly);
                bounds.width = maxX - bounds.lx;
                bounds.height = maxY - bounds.ly;
            }
        }
        return found && m_editorCamera -> frameBounds(bounds);
    }

    void ViewportPanel::instrumentBar() {
        ImGui::BeginGroup();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.f);
        ImGui::SetCursorPosX(6.f);
        const auto tooltip = [](const char* text) {
            if(ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("%s", text);
        };
        struct Tool {
            const char* icon;
            Guizmo2D::Operation operation;
            InstrumentMessage::Type message;
            const char* tooltip;
        };
        const Tool tools[] = {
            {ICON_FA_ARROWS_ALT, Guizmo2D::Operation::Move, InstrumentMessage::Type::Move, "Move (Shift+W)"},
            {ICON_FA_EXPAND_ARROWS_ALT, Guizmo2D::Operation::Scale, InstrumentMessage::Type::Scale, "Scale (Shift+Q)"},
            {ICON_FA_REDO_ALT, Guizmo2D::Operation::Rotate, InstrumentMessage::Type::Rotate, "Rotate (Shift+R)"}
        };
        for(const auto& tool: tools) {
            ImGui::PushID(static_cast<int>(tool.operation));
            const bool active = m_guizmo2D.getOperationType() == tool.operation;
            if(active)
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
            const float size = ImGui::GetFrameHeight();
            if(ImGui::Button(tool.icon, {size, size})) {
                m_guizmo2D.setOperationType(tool.operation);
                auto* message = m_messageBus.postMessage<InstrumentMessage>(MessageID::InstrumentPressed);
                message -> type = tool.message;
            }
            if(active)
                ImGui::PopStyleColor();
            tooltip(tool.tooltip);
            ImGui::PopID();
            ImGui::SameLine();
        }
        const auto nextControl = [](float width) {
            const float right = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
            if(ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x + width <= right)
                ImGui::SameLine();
            else
                ImGui::NewLine();
        };
        nextControl(85.f);
        const bool orthographic = m_editorCamera && m_editorCamera -> getType() == EditorCameraType::Orthographic;
        ImGui::BeginDisabled(!orthographic);
        ImGui::Checkbox(ICON_FA_TH " Grid", &m_gridVisible);
        tooltip("Show the viewport grid (independent from snapping)");
        nextControl(100.f);
        ImGui::BeginDisabled(m_guizmo2D.isActive());
        ImGui::Checkbox(ICON_FA_MAGNET " Snap", &m_snapEnabled);
        tooltip("Snap drag offsets for Move/Scale, and angle changes for Rotate");
        nextControl(36.f);
        if(ImGui::Button("...##SnapSettings"))
            ImGui::OpenPopup("Snap settings");
        if(ImGui::BeginPopup("Snap settings")) {
            ImGui::SetNextItemWidth(150.f);
            ImGui::DragFloat("Move / size step", &m_snapDistance, 1.f, 1.f, 1024.f, "%.0f",
                ImGuiSliderFlags_AlwaysClamp);
            ImGui::SetNextItemWidth(150.f);
            ImGui::DragFloat("Angle step", &m_snapAngle, 1.f, 1.f, 180.f, "%.0f deg", ImGuiSliderFlags_AlwaysClamp);
            ImGui::TextDisabled("Offsets from drag start; not absolute grid alignment.");
            ImGui::EndPopup();
        }
        m_guizmo2D.setSnapping(m_snapEnabled, m_snapDistance, m_snapAngle);
        nextControl(36.f);
        const bool hasSelection = m_uiInteractor && !m_uiInteractor -> getSelectedEntities().empty();
        ImGui::BeginDisabled(!hasSelection);
        if(ImGui::Button(ICON_FA_CROSSHAIRS "##FrameSelected"))
            frameEntities(false);
        tooltip("Frame Selected: center and fit the selected objects");
        ImGui::EndDisabled();
        nextControl(90.f);
        char zoomLabel[32];
        const float zoom = m_editorCamera ? m_editorCamera -> getZoom() : 1.f;
        std::snprintf(zoomLabel, sizeof(zoomLabel), "%.0f%%", static_cast<double>(zoom * 100.f));
        ImGui::SetNextItemWidth(90.f);
        if(ImGui::BeginCombo("##ViewportZoom", zoomLabel)) {
            for(float scale: {0.25f, 0.5f, 1.f, 2.f, 4.f, 8.f}) {
                char label[16];
                std::snprintf(label, sizeof(label), "%.0f%%", static_cast<double>(scale * 100.f));
                if(ImGui::Selectable(label, std::abs(zoom - scale) < 0.001f))
                    m_editorCamera -> setZoom(scale);
            }
            if(ImGui::Selectable("Frame scene"))
                frameEntities(true);
            ImGui::EndCombo();
        }
        tooltip("100% = one scene unit per viewport pixel. Ctrl+wheel zooms.");
        ImGui::EndDisabled();
        ImGui::EndDisabled();
        ImGui::EndGroup();
        ImGui::Separator();
    }

}
