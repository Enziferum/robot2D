#pragma once

#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Core/MessageBus.hpp>

#include "IPanel.hpp"

namespace editor {
    class TiledMapPanel: public IPanel {
    public:
        TiledMapPanel(robot2D::MessageBus& messageBus);
        ~TiledMapPanel() override = default;

        void render() override;
    private:
        void windowFunction();
        void tiledMapUI();
        void loadTextureUI();
    private:
        robot2D::MessageBus& m_messageBus;
        const robot2D::Texture* m_textureMap{nullptr};
        int m_tileSize = 1;
    };
}