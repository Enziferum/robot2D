#pragma once
#include <map>
#include <string>

#include <robot2D/Core/Vector2.hpp>
#include <robot2D/Graphics/Color.hpp>

#include <imgui/imgui.h>

namespace robot2D {
    struct WindowOptions {
        WindowOptions() = default;
        WindowOptions(std::map<int, robot2D::vec2f>&& variables,
                      std::map<int, robot2D::Color>&& colors);
        ~WindowOptions() = default;

        void enable() const;
        void disable() const;

        using WindowFlagsMask = int;
        std::map<int, robot2D::vec2f> m_vars;
        std::map<int, robot2D::Color> m_colors;

        WindowFlagsMask flagsMask = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar
                                    | ImGuiWindowFlags_NoScrollWithMouse;
        std::string name;
    };
} // namespace robot2D