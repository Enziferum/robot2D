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