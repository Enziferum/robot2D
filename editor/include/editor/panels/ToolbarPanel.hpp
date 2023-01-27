#pragma once
#include <robot2D/Core/MessageBus.hpp>
#include "IPanel.hpp"

namespace editor {
    class ToolbarPanel final: public IPanel {
    public:
        ToolbarPanel(robot2D::MessageBus& messageBus);
        ~ToolbarPanel() override = default;
        void render() override;
    private:
        robot2D::MessageBus& m_messageBus;
    };
}