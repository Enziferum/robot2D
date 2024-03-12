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