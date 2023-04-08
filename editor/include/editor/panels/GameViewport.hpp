#pragma once
#include <robot2D/Core/MessageBus.hpp>
#include "IPanel.hpp"

namespace editor {
    class GameViewport final: public IPanel {
    public:
        GameViewport(robot2D::MessageBus& messageBus);
        ~GameViewport() override = default;


        void render() override;
    private:
        robot2D::MessageBus& m_messageBus;
    };
}