#pragma once

#include <string>
#include <imgui/imgui.h>
#include <robot2D/Graphics/Color.hpp>

namespace robot2D {

    class Spinner {
    public:
        Spinner() = default;
        ~Spinner() = default;

        void setPosition(const robot2D::vec2f& position) {
            ImGui::SetCursorPos(position);
        }

        void setRadius(float radius) { m_radius = radius; }
        void setThickness(float thickness) { m_thickness = thickness; }
        void setLabel(const std::string& label) { m_label = label; }
        void setSpeed(float speed) { m_speed = speed; }
        void setAngle(float angle) { m_angle = angle; }
        void setColor(robot2D::Color color) { m_color = color; }
        void setBackgroundColor(robot2D::Color color) { m_backgroundColor = color; }

        // TODO(a.raag): calculate size over whole spinner bb
        robot2D::vec2f calculateSize(const ImGuiStyle& style) {
            return {(m_radius + m_thickness) * 2, (m_radius + m_thickness + style.FramePadding.y) * 2};
        }

        void setIsShow(bool flag) { m_isShowing = flag; }
        bool isShowing() const {
            return m_isShowing;
        }

        void draw();
    private:
        bool begin();
    private:
        bool m_isShowing{false};
        robot2D::vec2f m_center;

        std::string m_label;
        float m_radius;
        float m_thickness;
        float m_speed;
        float m_angle;
        robot2D::Color m_color;
        robot2D::Color m_backgroundColor;
    };
}