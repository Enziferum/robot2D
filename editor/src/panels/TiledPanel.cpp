#include <editor/panels/TiledPanel.hpp>
#include <robot2D/imgui/Api.hpp>

namespace editor {


    TiledPanel::TiledPanel():
        IPanel(typeid(TiledPanel)),
        m_state(TiledPanelState::Default)
    {

    }

    void TiledPanel::render() {
        imgui_Window("TiledPanel") {
            switch(m_state) {
                case TiledPanelState::Default: {
                    drawBase();
                    break;
                }
                case TiledPanelState::Tiled: {
                    drawTiled();
                    break;
                }
            }
        }
    }

    void TiledPanel::drawBase() {

    }

    void TiledPanel::drawTiled() {

    }

} // namespace editor