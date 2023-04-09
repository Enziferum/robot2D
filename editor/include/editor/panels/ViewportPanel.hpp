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
#include <editor/UIManager.hpp>
#include <editor/EditorCamera.hpp>
#include <editor/Guizmo2D.hpp>
#include <editor/Collider.hpp>
#include <editor/MessageDispather.hpp>

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
        ViewportPanel(IUIManager& uiManager,
                      IEditorCamera::Ptr editorCamera,
                      robot2D::MessageBus& messageBus,
                      MessageDispatcher& messageDispatcher,
                      Guizmo2D& guizmo2D,
                      Collider& collider);
        ~ViewportPanel() override = default;

        void handleEvents(const robot2D::Event& event);
        void set(robot2D::FrameBuffer::Ptr frameBuffer);

        void update(float deltaTime) override;
        void render() override;
    private:
        void instrumentBar(robot2D::vec2f windowOffset, robot2D::vec2f windowAvailSize);
        void toolbarOverlay(robot2D::vec2f windowOffset, robot2D::vec2f windowAvailSize);
    private:
        IUIManager& m_uiManager;
        IEditorCamera::Ptr m_editorCamera;
        robot2D::MessageBus& m_messageBus;
        MessageDispatcher& m_messageDispatcher;
        Collider& m_CameraCollider;

        robot2D::ResourceHandler<robot2D::Texture, IconType> m_icons;

        robot2D::FrameBuffer::Ptr m_frameBuffer;
        robot2D::vec2u  m_ViewportSize{};
        robot2D::WindowOptions m_windowOptions;

        robot2D::vec2f m_ViewportBounds[2];

        bool m_panelHovered;
        bool m_panelFocused;
        bool needResetViewport{false};

        Guizmo2D& m_guizmo2D;
        robot2D::ecs::Entity m_selectedEntity;
    };
}