#pragma once

#include "IPanel.hpp"

namespace editor {

    enum class TiledPanelState {
        Default,
        Tiled
    };


    class TiledPanel final: public IPanel {
    public:
        TiledPanel();
        TiledPanel(const TiledPanel& other) = delete;
        TiledPanel& operator=(const TiledPanel& other) = delete;
        TiledPanel(TiledPanel&& other) = delete;
        TiledPanel& operator=(TiledPanel&& other) = delete;
        ~TiledPanel() override = default;

        void render() override;
    private:
        void drawBase();
        void drawTiled();

    private:
        TiledPanelState m_state;
    };
} // namespace editor