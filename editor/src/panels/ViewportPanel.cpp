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

#include <robot2D/imgui/Api.hpp>
#include <robot2D/imgui/DragDropTarget.hpp>
#include <imgui/imgui.h>

#include <editor/panels/ViewportPanel.hpp>
#include <editor/Messages.hpp>
#include <editor/Components.hpp>
#include <editor/DragDropIDS.hpp>
#include <editor/Buffer.hpp>

namespace editor {
    namespace {
        std::string iconPath = "res/icons";

        static std::unordered_map<IconType, std::string> iconNames = {
                { IconType::Play, "PlayButton.png" },
                { IconType::Stop, "StopButton.png" },
                { IconType::Pause, "PauseButton.png" },
                { IconType::Step, "StepButton.png" },
                { IconType::Simulate, "SimulateButton.png" },
                { IconType::Move, "move.png" },
                { IconType::Scale, "scale.png" },
                { IconType::Rotate, "rotate.png" },
        };

        constexpr int frameBufferAttachmentIndex = 1;
    }

    ViewportPanel::ViewportPanel(
            UIInteractor* uiInteractor,
            IEditorCamera::Ptr editorCamera,
            robot2D::MessageBus& messageBus,
            MessageDispatcher& messageDispatcher,
            Guizmo2D& guizmo2D,
            CameraManipulator& collider,
            SelectionCollider& selectionCollider):
            IPanel(UniqueType(typeid(ViewportPanel))),
            m_uiInteractor{uiInteractor},
            m_editorCamera{editorCamera},
            m_messageBus{messageBus},
            m_messageDispatcher{messageDispatcher},
            m_selectionCollider{selectionCollider},
            m_guizmo2D{guizmo2D},
            m_CameraCollider{collider}
    {
        m_windowOptions = robot2D::WindowOptions {
            {
                {ImGuiStyleVar_WindowPadding, {0, 0}}
            },
            {}
        };
        m_windowOptions.flagsMask = ImGuiWindowFlags_NoScrollbar;
        m_windowOptions.name = "Viewport";

        for(const auto& [type, name]: iconNames) {
            std::filesystem::path path{iconPath};
            path.append(name);
            if(!m_icons.loadFromFile(type, path.string())) {
                RB_EDITOR_ERROR("Can't load icon");
                // TODO(a.raag) throw exception
            }
        }

    }

    void ViewportPanel::setFramebuffer(robot2D::FrameBuffer::Ptr frameBuffer) {
        m_frameBuffer = std::move(frameBuffer);
        needResetViewport = true;
    }


    void ViewportPanel::handleEvents(const robot2D::Event& event) {
        if(event.type == robot2D::Event::MousePressed && m_panelFocused && m_panelHovered) {
            auto[mx, my] = ImGui::GetMousePos();
            mx -= m_ViewportBounds.minRegion.x;
            my -= m_ViewportBounds.minRegion.y;
            robot2D::vec2f viewportSize = m_ViewportBounds.maxRegion - m_ViewportBounds.minRegion;
            my = viewportSize.y - my;

            if( !m_guizmo2D.isActive() && !m_CameraCollider.isActive() ) {
                m_frameBuffer -> Bind();
                int graphicsEntityID = m_frameBuffer -> readPixel(frameBufferAttachmentIndex,
                                                                  { static_cast<int>(mx), static_cast<int>(my) });
                m_frameBuffer -> unBind();
                m_uiInteractor -> getSelectedEntity(graphicsEntityID);
            }
        }
    }

    void ViewportPanel::update(float deltaTime) {
        auto[mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds.minRegion.x;
        my -= m_ViewportBounds.minRegion.y;

        robot2D::vec2f viewportSize = m_ViewportBounds.maxRegion - m_ViewportBounds.minRegion;
        my = viewportSize.y - my;

        if(m_panelFocused && m_panelHovered) {
            m_editorCamera -> update({mx, my}, deltaTime);
        }
    }

    void ViewportPanel::render() {
        robot2D::createWindow(m_windowOptions, [this]() {
            auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
            auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
            auto viewportOffset = ImGui::GetWindowPos();
            m_ViewportBounds.minRegion = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
            m_ViewportBounds.maxRegion = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

            m_panelFocused = ImGui::IsWindowFocused();
            m_panelHovered = ImGui::IsWindowHovered();

            auto ViewPanelSize = ImGui::GetContentRegionAvail();

            if (ImGui::IsWindowDocked()) {
                toolbarOverlay({ viewportOffset.x, viewportOffset.y }, { ViewPanelSize.x, ViewPanelSize.y });
                instrumentBar({ viewportOffset.x, viewportOffset.y }, { ViewPanelSize.x, ViewPanelSize.y });
            }
            m_editorCamera -> setViewportBounds({viewportOffset.x, viewportOffset.y
                                                                   + (viewportMaxRegion.y - ViewPanelSize.y)});
            if(m_ViewportSize != robot2D::vec2u { static_cast<unsigned int>(ViewPanelSize.x), 
                        static_cast<unsigned int>(ViewPanelSize.y)} || needResetViewport) {
                m_ViewportSize = {static_cast<unsigned int>(ViewPanelSize.x),
                                  static_cast<unsigned int>(ViewPanelSize.y)};
                m_frameBuffer -> Resize(m_ViewportSize);
                m_editorCamera -> setViewportSize(m_ViewportSize.as<float>());
                needResetViewport = false;
            }

            robot2D::RenderFrameBuffer(m_frameBuffer, m_ViewportSize.as<float>());

            m_guizmo2D.setIsShow(false);
            m_CameraCollider.setIsShownDots(false);

            auto& selectedEntities = m_uiInteractor -> getSelectedEntities();

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
                    }
                }
            }
            else if(selectedEntities.size() > 1) {
                std::vector<TransformComponent*> moveTfs;
                for(auto& entity: selectedEntities) {
                   if(entity) {
                       auto &tc = entity.getComponent<TransformComponent>();
                       moveTfs.emplace_back(&tc);
                   }
                }
                m_guizmo2D.setIsShow(true);
                m_guizmo2D.setManipulated(moveTfs);
            }
            else {
                m_guizmo2D.setManipulated(nullptr);
            }

            {
                robot2D::DragDropTarget dragDropTarget{};
                if(auto&& payloadBuffer = dragDropTarget.unpackPayload2Buffer(contentSceneID,
                                                                          ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
                    auto&& path = payloadBuffer.unpack<std::string>();
                    std::filesystem::path scenePath = std::filesystem::path("assets") / path;

                    auto messagePath = scenePath.string();
                    int allocSize = StringBuffer::calcAllocSize(messagePath);
                    void* rawBuffer = m_messageBus.postMessage(MessageID::OpenScene, allocSize);
                    Buffer buffer { rawBuffer };
                    pack_message_string(messagePath, buffer);
                }

                if(auto&& payloadBuffer = dragDropTarget.unpackPayload2Buffer(contentPrefabItemID,
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
    }


    void ViewportPanel::toolbarOverlay(robot2D::vec2f windowOffset, robot2D::vec2f windowAvailSize) {
        static ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking
                | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
                | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_NoScrollbar;
        static bool p_open = true;
        constexpr auto buttonSize = ImVec2{20, 20};

        {
            const ImGuiViewport* viewport = ImGui::GetWindowViewport();
            ImVec2 window_pos;

            window_pos = { ( windowAvailSize.x - 100) / 2, 40};
            window_pos.x += windowOffset.x;
            window_pos.y += windowOffset.y;

            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
            ImGui::SetNextWindowViewport(viewport->ID);
            window_flags |= ImGuiWindowFlags_NoMove;
        }


        {
            robot2D::ScopedStyleVarF scopedStyleVars{{ImGuiStyleVar_WindowBorderSize, 0.0f},
                                                           {ImGuiStyleVar_WindowRounding, 8.f}};

            imgui_Window("##Toolbar", &p_open, window_flags) {
                auto contentSize = ImGui::GetContentRegionAvail();

                ImGui::SetCursorPos({(contentSize.x - buttonSize.x) / 2.F, (contentSize.y - buttonSize.y) / 2.F});
                IconType iconType;

                if(m_uiInteractor)
                    iconType = m_uiInteractor -> isRunning() ? IconType::Stop : IconType::Play;
                else
                    iconType = IconType::Play;
                if(robot2D::ImageButton(m_icons[iconType], { buttonSize.x, buttonSize.y })) {
                    auto* msg = m_messageBus.postMessage<ToolbarMessage>(MessageID::ToolbarPressed);
                    msg -> pressedType = m_uiInteractor -> isRunning() ? 0 : 1;
                }
            }
        }
    }

    void ViewportPanel::instrumentBar(robot2D::vec2f windowOffset, robot2D::vec2f windowAvailSize) {
        static ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking
                | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
                | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_NoScrollbar;
        static bool p_open = true;
        constexpr auto buttonSize = ImVec2{ 30, 30 };
        {
            const ImGuiViewport* viewport = ImGui::GetWindowViewport();
            ImVec2 window_pos { 40, 40 };
            window_pos.x += windowOffset.x;
            window_pos.y += windowOffset.y;

            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
            ImGui::SetNextWindowViewport(viewport -> ID);
            window_flags |= ImGuiWindowFlags_NoMove;
        }

        {
            robot2D::ScopedStyleVarF scopedStyleVars{{ImGuiStyleVar_WindowBorderSize, 1.0f},
                                                           {ImGuiStyleVar_WindowRounding, 8.f}};
            ImGui::Columns(1, nullptr, false);
            imgui_Window("##InstrumentBar", &p_open, window_flags) {
                auto contentSize = ImGui::GetContentRegionAvail();

                robot2D::ScopedStyleColor scopedStyleColor(ImGuiCol_Button, robot2D::Color(255.f, 255.f, 255.f, 127.f));
                ImGui::SetCursorPosX((contentSize.x - buttonSize.x) / 2.F);
                if(robot2D::ImageButton(m_icons[IconType::Move], { buttonSize.x, buttonSize.y })) {
                    auto* msg = m_messageBus.postMessage<InstrumentMessage>(MessageID::InstrumentPressed);
                    msg -> type = InstrumentMessage::Type::Move;

                    m_guizmo2D.setOperationType(Guizmo2D::Operation::Move);
                }
                ImGui::SetCursorPosX((contentSize.x - buttonSize.x) / 2.F);
                if(robot2D::ImageButton(m_icons[IconType::Scale], { buttonSize.x, buttonSize.y })) {
                    auto* msg = m_messageBus.postMessage<InstrumentMessage>(MessageID::InstrumentPressed);
                    msg -> type = InstrumentMessage::Type::Scale;
                    m_guizmo2D.setOperationType(Guizmo2D::Operation::Scale);
                }
                ImGui::SetCursorPosX((contentSize.x - buttonSize.x) / 2.F);
                if(robot2D::ImageButton(m_icons[IconType::Rotate], { buttonSize.x, buttonSize.y })) {
                    auto* msg = m_messageBus.postMessage<InstrumentMessage>(MessageID::InstrumentPressed);
                    msg -> type = InstrumentMessage::Type::Rotate;
                    m_guizmo2D.setOperationType(Guizmo2D::Operation::Rotate);
                }
            }
        }
    }

}
