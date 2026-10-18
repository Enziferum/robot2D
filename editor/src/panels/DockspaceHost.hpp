#pragma once

#include <functional>
#include <robot2D/imgui/WindowScope.hpp>

namespace editor::ui {
    /// Keep the same host ID stack when content is skipped by Begin.
    inline void renderDockspaceHost(const robot2D::WindowOptions& options,
        const char* dockspaceName, ImGuiDockNodeFlags flags,
        const std::function<void()>& content) {
        robot2D::WindowScope window{options};
        if(window)
            content();
        else if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
            ImGui::DockSpace(ImGui::GetID(dockspaceName), {0.f, 0.f},
                flags | ImGuiDockNodeFlags_KeepAliveOnly);
    }
}
