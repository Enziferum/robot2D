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

#include <robot2D/imgui/WindowOptions.hpp>
#include <imgui/imgui.h>

namespace robot2D {
    WindowOptions::WindowOptions(std::map<int, robot2D::vec2f>&& variables,
                                 std::map<int, robot2D::Color>&& colors):
                                 m_vars{std::move(variables)},
                                 m_colors{std::move(colors)}
                                 {}


    void WindowOptions::enable() const {
        for (const auto& var: m_vars)
            ImGui::PushStyleVar(var.first, {var.second.x, var.second.y});
        for (const auto& var: m_colors) {
            auto glColor = var.second.toGL();
            ImGui::PushStyleColor(var.first, ImVec4(glColor.red,
                                             glColor.green,
                                             glColor.blue,
                                             glColor.alpha));
        }
    }

    void WindowOptions::disable() const {
        ImGui::PopStyleVar(static_cast<int>(m_vars.size()));
        ImGui::PopStyleColor(static_cast<int>(m_colors.size()));
    }
} // namespace robot2D