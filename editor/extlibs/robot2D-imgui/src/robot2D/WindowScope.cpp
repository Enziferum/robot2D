#include <robot2D/imgui/WindowScope.hpp>

namespace robot2D {
    WindowStyleScope::WindowStyleScope(const WindowOptions& options) :
        m_variables(static_cast<int>(options.m_vars.size())),
        m_colors(static_cast<int>(options.m_colors.size())) {
        options.enable();
    }

    WindowStyleScope::~WindowStyleScope() {
        ImGui::PopStyleColor(m_colors);
        ImGui::PopStyleVar(m_variables);
    }

    WindowScope::WindowScope(const WindowOptions& options, bool* open) :
        m_style(options),
        m_visible(ImGui::Begin(options.name.c_str(), open, options.flagsMask)) {
        m_visible = m_visible && (!open || *open);
    }

    WindowScope::~WindowScope() {
        ImGui::End();
    }
}
