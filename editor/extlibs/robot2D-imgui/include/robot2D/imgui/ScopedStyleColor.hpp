#pragma once

#include <initializer_list>
#include <utility>

#include <imgui/imgui.h>
#include <robot2D/Graphics/Color.hpp>

namespace robot2D {

    struct ScopedStyleColor {
        ScopedStyleColor(int rawIdx, robot2D::Color color) {
            ImGui::PushStyleColor(rawIdx, color);
            m_count = 1;
        }

        ScopedStyleColor(const std::initializer_list<std::pair<int, robot2D::Color>>& colors) {
            for(const auto& color: colors) {
                ImGui::PushStyleColor(color.first, color.second);
            }
            m_count = colors.size();
        }

        ~ScopedStyleColor() { ImGui::PopStyleColor(m_count); }
    private:
        int m_count;
    };

}