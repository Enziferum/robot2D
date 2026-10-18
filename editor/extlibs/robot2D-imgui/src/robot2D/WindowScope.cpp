#include <robot2D/imgui/WindowScope.hpp>
#include <imgui/imgui_internal.h>

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

    WindowScope::WindowScope(const WindowOptions& options, bool* open) {
        if(open && !*open) {
            // Begin normally consumes these; do not leak a closed window's
            // position, size, focus or docking request to the next window.
            ImGui::GetCurrentContext() -> NextWindowData.ClearFlags();
            return;
        }
        m_style.emplace(options);
        m_visible = ImGui::Begin(options.name.c_str(), open, options.flagsMask);
        m_begun = true;
        m_visible = m_visible && (!open || *open);
    }

    WindowScope::~WindowScope() {
        if(m_begun)
            ImGui::End();
    }
}
