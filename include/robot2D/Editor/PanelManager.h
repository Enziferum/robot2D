//
// Created by support on 26.03.2021.
//

#pragma once
#include <unordered_map>
#include <robot2D/Core/Event.h>

#include "IPanel.h"

namespace robot2D{
    class PanelManager{
    public:
        PanelManager();
        ~PanelManager() = default;

        void handleEvents(const Event& );
        void update(float dt);

        template<typename T, typename ... Args>
        void registerPanel(Args&& ... args);
    private:
        int32_t m_current = 0;
        std::unordered_map<int32_t, IPanel::Ptr> m_panels;
    };

    template<typename T, typename ... Args>
    void PanelManager::registerPanel(Args&& ... args) {
        auto panel = std::make_shared<T>(std::forward<Args>(args)...);
        if(panel == nullptr)
            return;
        m_panels.insert(std::pair<int32_t, IPanel::Ptr>(m_current, panel));
        ++m_current;
    }


}
