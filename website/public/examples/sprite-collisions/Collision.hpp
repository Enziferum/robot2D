// Core lesson 06. Zlib license; see LICENSE.txt.
#pragma once

#include <algorithm>
#include <cmath>
#include <robot2D/Graphics/Sprite.hpp>

// Positive sizes, normalized origins, zero rotation and default scale only.
inline robot2D::FloatRect quadBounds(robot2D::Sprite& sprite) {
    const auto position = sprite.getPosition();
    const auto size = sprite.getSize();
    const auto origin = sprite.getOrigin();
    return {position.x - origin.x * size.x, position.y - origin.y * size.y, size.x, size.y};
}

inline bool overlaps(const robot2D::FloatRect& a, const robot2D::FloatRect& b) {
    return a.lx < b.lx + b.width && a.lx + a.width > b.lx &&
           a.ly < b.ly + b.height && a.ly + a.height > b.ly;
}

struct CollisionResult {
    bool blockedX = false;
    bool blockedY = false;
};

// Start outside all solids. Sweep X, then sweep Y from the resolved X position.
// This is a small kinematic solver for one player and static rectangular walls.
template<typename Walls>
CollisionResult moveAndSlide(robot2D::Sprite& player, robot2D::vec2f movement, Walls& walls) {
    CollisionResult result;
    if(!std::isfinite(movement.x) || !std::isfinite(movement.y)) return result;
    auto bounds = quadBounds(player);
    float dx = movement.x;
    for(auto& wall : walls) {
        const auto solid = quadBounds(wall);
        if(bounds.ly >= solid.ly + solid.height || bounds.ly + bounds.height <= solid.ly) continue;
        if(dx > 0.F && bounds.lx + bounds.width <= solid.lx)
            dx = std::min(dx, solid.lx - (bounds.lx + bounds.width));
        else if(dx < 0.F && bounds.lx >= solid.lx + solid.width)
            dx = std::max(dx, solid.lx + solid.width - bounds.lx);
    }
    result.blockedX = dx != movement.x;
    player.move({dx, 0.F});

    bounds = quadBounds(player);
    float dy = movement.y;
    for(auto& wall : walls) {
        const auto solid = quadBounds(wall);
        if(bounds.lx >= solid.lx + solid.width || bounds.lx + bounds.width <= solid.lx) continue;
        if(dy > 0.F && bounds.ly + bounds.height <= solid.ly)
            dy = std::min(dy, solid.ly - (bounds.ly + bounds.height));
        else if(dy < 0.F && bounds.ly >= solid.ly + solid.height)
            dy = std::max(dy, solid.ly + solid.height - bounds.ly);
    }
    result.blockedY = dy != movement.y;
    player.move({0.F, dy});
    return result;
}
