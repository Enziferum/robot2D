//
// Created by Necromant on 03.10.2021.
//

#pragma once
#include <vector>
#include "IPanel.hpp"

namespace editor {
    class PanelManager {
    public:
        PanelManager();
        ~PanelManager() = default;

        template<typename T, typename ...Args>
        T& addPanel(Args&& ...args);

        template<typename T>
        T& getPanel();

        void update(float dt);
        void render();
    private:
        std::vector<IPanel::Ptr> m_panels;
    };

    template<typename T, typename ...Args>
    T& PanelManager::addPanel(Args&& ...args) {
        auto& panel = m_panels.emplace_back(std::shared_ptr<T>(std::forward<Args>(args)...));
        return *(dynamic_cast<T*>(m_panels.back().get()));
    }

#include <algorithm>

    template<typename T>
    T& PanelManager::getPanel() {
        UniqueType uniqueType(typeid(T));
        auto found = std::find_if(m_panels.begin(), m_panels.end(), [uniqueType](const IPanel::Ptr& ptr) {
            return ptr->getID() == uniqueType;
        });

        return *(dynamic_cast<T*>(found->get()));
    }
}
