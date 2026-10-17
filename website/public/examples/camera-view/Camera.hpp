#pragma once

#include <algorithm>
#include <robot2D/Graphics/View.hpp>

inline void followPlayer(robot2D::View& view, robot2D::vec2f player, robot2D::vec2u logicalSize) {
    if(!logicalSize.x || !logicalSize.y) return;
    // Fixed vertical field of view; resizing changes how much world is visible horizontally.
    const float height = 600.F;
    const float width = height * static_cast<float>(logicalSize.x) / logicalSize.y;
    const auto center = [](float position, float visible, float world) {
        return visible >= world ? world * 0.5F : std::clamp(position, visible * 0.5F, world - visible * 0.5F);
    };
    view.reset({0.F, 0.F, width, height});
    view.setCenter({center(player.x, width, 2000.F), center(player.y, height, 1200.F)});
}
