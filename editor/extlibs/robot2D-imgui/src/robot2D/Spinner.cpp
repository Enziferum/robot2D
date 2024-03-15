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

#ifdef _WIN32
    #define _USE_MATH_DEFINES
#endif 
#include <cmath>

#include <robot2D/imgui/Spinner.hpp>
#include <imgui/imgui_internal.h>

namespace robot2D {

    void Spinner::draw() {
        if(!begin())
            return;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        window -> DrawList -> PathClear();

        const size_t num_segments = window -> DrawList->_CalcCircleAutoSegmentCount(m_radius);
        float start = (float)ImGui::GetTime() * m_speed;
        const float bg_angle_offset = M_PI * 2.f / num_segments;

        for (size_t i = 0; i <= num_segments; i++)
        {
            const float a = start + (i * bg_angle_offset);
            window -> DrawList -> PathLineTo(ImVec2(m_center.x + std::cos(a) * m_radius,
                                                    m_center.y + std::sin(a) * m_radius));
        }
        window -> DrawList -> PathStroke(ImColor(m_backgroundColor), false, m_thickness);

        window -> DrawList-> PathClear();
        const float angle_offset = m_angle / num_segments;
        for (size_t i = 0; i < num_segments; i++)
        {
            const float a = start + (i * angle_offset);
            window -> DrawList -> PathLineTo(ImVec2(m_center.x + std::cos(a) * m_radius, m_center.y + std::sin(a) * m_radius));
        }
        window -> DrawList-> PathStroke(ImColor(m_color), false, m_thickness);
    }

    bool Spinner::begin() {
        ImVec2 pos, size;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window -> SkipItems)
            return false;

        ImGuiContext* g = ImGui::GetCurrentContext();
        const ImGuiStyle& style = g -> Style;
        const ImGuiID id = window -> GetID(m_label.c_str());

        pos = window -> DC.CursorPos;
        size = ImVec2((m_radius) * 2, (m_radius + style.FramePadding.y) * 2);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);

        m_center = bb.GetCenter();
        if (!ImGui::ItemAdd(bb, id))
            return false;

        return true;
    }

}