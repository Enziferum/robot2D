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

#include <initializer_list>
#include <utility>

#include <imgui/imgui.h>
#include <robot2D/Graphics/Color.hpp>

namespace robot2D {

    /// \brief RAII wrapper ImGui StyleColor
    struct ScopedStyleColor {
        ScopedStyleColor() = delete;
        ScopedStyleColor(const ScopedStyleColor& other) = delete;
        ScopedStyleColor& operator=(const ScopedStyleColor& other) = delete;
        ScopedStyleColor(ScopedStyleColor&& other) = delete;
        ScopedStyleColor& operator=(ScopedStyleColor&& other) = delete;

        ScopedStyleColor(int rawIdx, robot2D::Color color): m_count { 1 } {
            ImGui::PushStyleColor(rawIdx, color);
        }

        ScopedStyleColor(const std::initializer_list<std::pair<int, robot2D::Color>>& colors) {
            for(const auto& color: colors)
                ImGui::PushStyleColor(color.first, color.second);
            m_count = colors.size();
        }

        ~ScopedStyleColor() { ImGui::PopStyleColor(m_count); }
    private:
        int m_count { 0 };
    };

} // namespace robot2D