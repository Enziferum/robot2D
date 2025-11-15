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

namespace editor {
    struct UIHitbox {
        enum CallbackID {
            MouseUp = 1,
            MouseDown = 2,
            MouseMoved = 3,
            Count
        };

        std::array<std::int32_t, CallbackID::Count> callbackIDs{-1, -1, -1, -1};

        bool onHoverOnce = true;
        bool onUnHoverOnce = true;

        bool wasHovered = false;

        bool isHovered{false};

        bool contains(const robot2D::vec2f& position) {
            return m_area.contains(position);
        }

        robot2D::FloatRect m_area;
    };
} // namespace editor