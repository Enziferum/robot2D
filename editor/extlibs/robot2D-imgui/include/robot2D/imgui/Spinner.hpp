/*********************************************************************
(c) Alex Raag 2024
https://github.com/Enziferum
robot2D - Zlib license.
This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#pragma once

#include <string>
#include <imgui/imgui.h>
#include <robot2D/Graphics/Color.hpp>

namespace robot2D {

    class Spinner {
    public:
        Spinner() = default;
        Spinner(const Spinner& other) = delete;
        Spinner& operator=(const Spinner& other) = delete;
        Spinner(Spinner&& other) = delete;
        Spinner& operator=(Spinner&& other) = delete;
        ~Spinner() = default;

        void setPosition(const robot2D::vec2f& position) {
            m_position = position;
            ImGui::SetCursorPos(position);
        }

        [[nodiscard]]
        robot2D::vec2f getPosition() const noexcept {
            return m_position;
        }

        void setRadius(float radius) { m_radius = radius; }
        void setThickness(float thickness) { m_thickness = thickness; }
        void setLabel(const std::string& label) { m_label = label; }
        void setSpeed(float speed) { m_speed = speed; }
        void setAngle(float angle) { m_angle = angle; }
        void setColor(robot2D::Color color) { m_color = color; }
        void setBackgroundColor(robot2D::Color color) { m_backgroundColor = color; }

        // TODO(a.raag): calculate size over whole spinner bb
        [[nodiscard]]
        robot2D::vec2f calculateSize(const ImGuiStyle& style) const {
            return {(m_radius + m_thickness) * 2, (m_radius + m_thickness + style.FramePadding.y) * 2};
        }

        void setIsShow(bool flag) { m_isShowing = flag; }
        bool isShowing() const noexcept { return m_isShowing; }

        void draw();
    private:
        bool begin();
    private:
        bool m_isShowing{false};
        robot2D::vec2f m_center { };
        robot2D::vec2f m_position{ };

        std::string m_label;
        float m_radius { 0.f };
        float m_thickness { 0.f };
        float m_speed { 0.f };
        float m_angle { 0.f };
        robot2D::Color m_color { robot2D::Color::White };
        robot2D::Color m_backgroundColor { robot2D::Color::White };
    };
} // namespace robot2D