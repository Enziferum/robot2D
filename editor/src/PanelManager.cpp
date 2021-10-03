//
// Created by Necromant on 03.10.2021.
//

#include <editor/PanelManager.hpp>

namespace editor {
    PanelManager::PanelManager(): m_panels() {}

    void PanelManager::update(float dt) {
        for(auto& it: m_panels)
            it -> update(dt);
    }

    void PanelManager::render() {
        for(auto& it: m_panels)
            it -> render();
    }
}