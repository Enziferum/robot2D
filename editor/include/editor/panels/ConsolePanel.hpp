#pragma once

#include <array>
#include <imgui/imgui.h>
#include <editor/ConsoleLog.hpp>
#include "IPanel.hpp"

namespace editor {

    class ConsolePanel final: public IPanel {
    public:
        explicit ConsolePanel(std::string path);
        ConsolePanel(const ConsolePanel& other) = delete;
        ConsolePanel& operator=(const ConsolePanel& other) = delete;
        ConsolePanel(ConsolePanel&& other) = delete;
        ConsolePanel& operator=(ConsolePanel&& other) = delete;
        ~ConsolePanel() override = default;

        void update(float deltaTime) override;
        void render() override;
    private:
        void renderEngineLog();
        ConsoleLog m_log;
        ImGuiTextFilter m_filter;
        std::array<bool, 7> m_levels {{true, true, true, true, true, true, true}};
        bool m_autoScroll {true};
        float m_pollTime {0.f};
        std::uint64_t m_renderedVersion {0};
    };

}
