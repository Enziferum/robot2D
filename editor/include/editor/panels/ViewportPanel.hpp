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

#pragma once

#include <utility>
#include <robot2D/Graphics/FrameBuffer.hpp>
#include <robot2D/imgui/Gui.hpp>
#include <robot2D/Core/MessageBus.hpp>
#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Util/ResourceHandler.hpp>

#include <editor/Scene.hpp>
#include <editor/UIInteractor.hpp>
#include <editor/EditorCamera.hpp>
#include <editor/Guizmo2D.hpp>
#include <editor/CameraManipulator.hpp>
#include <editor/MessageDispather.hpp>
#include <editor/SelectionCollider.hpp>

#include "IPanel.hpp"

namespace editor {

    enum class IconType {
        Play,
        Stop,
        Pause,
        Step,
        Simulate,
        Move,
        Scale,
        Rotate
    };

    class ViewportPanel final: public IPanel {
    public:
        ViewportPanel(UIInteractor* uiInteractor,
                      IEditorCamera::Ptr editorCamera,
                      robot2D::MessageBus& messageBus,
                      MessageDispatcher& messageDispatcher,
                      Guizmo2D& guizmo2D,
                      CameraManipulator& collider,
                      SelectionCollider& selectionCollider);
        ViewportPanel(const ViewportPanel& other) = delete;
        ViewportPanel& operator=(const ViewportPanel& other) = delete;
        ViewportPanel(ViewportPanel&& other) = delete;
        ViewportPanel& operator=(ViewportPanel&& other) = delete;
        ~ViewportPanel() override = default;

        bool isActive() const noexcept {
            return m_panelHovered && m_panelFocused;
        }

        void handleEvents(const robot2D::Event& event);
        void setFramebuffer(robot2D::FrameBuffer::Ptr frameBuffer);


        void update(float deltaTime) override;
        void render() override;
    private:
        void instrumentBar(robot2D::vec2f windowOffset, robot2D::vec2f windowAvailSize);
        void toolbarOverlay(robot2D::vec2f windowOffset, robot2D::vec2f windowAvailSize);
    private:
        UIInteractor* m_uiInteractor;

        IEditorCamera::Ptr m_editorCamera;
        robot2D::MessageBus& m_messageBus;
        MessageDispatcher& m_messageDispatcher;
        CameraManipulator& m_CameraCollider;
        SelectionCollider& m_selectionCollider;
        Guizmo2D& m_guizmo2D;

        robot2D::ResourceHandler<robot2D::Texture, IconType> m_icons;

        std::vector<robot2D::ecs::Entity> m_selectedEntities;
        robot2D::FrameBuffer::Ptr m_frameBuffer{ nullptr };
        robot2D::vec2u  m_ViewportSize{};
        robot2D::WindowOptions m_windowOptions;

        struct ViewportBounds {
            robot2D::vec2f minRegion { };
            robot2D::vec2f maxRegion { };
        } m_ViewportBounds;

        bool m_panelHovered { false };
        bool m_panelFocused { false };
        bool needResetViewport{false};
    };
}