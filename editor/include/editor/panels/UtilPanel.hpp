#pragma once

#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Graphics/RenderStats.hpp>

#include "IPanel.hpp"
#include "editor/EditorCamera.hpp"

namespace editor {
    struct UtilPanelConfiguration {
        const robot2D::Color defaultBackGround = robot2D::Color::fromGL(59.f/255.f, 60.f/255.f, 57.f/255.f, 1);
    };

    class UtilPanel: public IPanel {
    public:
        explicit UtilPanel(IEditorCamera::Ptr sceneCamera);
        ~UtilPanel() override = default;

        const robot2D::Color& getColor() const;

        void setRenderStats(robot2D::RenderStats&& renderStats);
        void render() override;
    private:
        IEditorCamera::Ptr m_camera;
        robot2D::Color m_clearColor;
        robot2D::RenderStats m_renderStats;
        UtilPanelConfiguration m_configuration;
    };
}