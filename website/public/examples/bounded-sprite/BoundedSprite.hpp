// Core lesson 03. Zlib license, as in lesson 01.
#pragma once

#include "../keyboard-movement/MovingSprite.hpp"
#include <algorithm>

// Positive size, normalized origin, no rotation or additional scale.
inline void keepInside(robot2D::Sprite& sprite, robot2D::vec2f area) {
    if(area.x <= 0.F || area.y <= 0.F) return;
    const auto size = sprite.getSize();
    const auto origin = sprite.getOrigin();
    const auto axis = [](float position, float extent, float length, float anchor) {
        if(length > extent) return extent * 0.5F + (anchor - 0.5F) * length;
        return std::clamp(position, anchor * length, extent - (1.F - anchor) * length);
    };
    const auto position = sprite.getPosition();
    sprite.setPosition({axis(position.x, area.x, size.x, origin.x),
                        axis(position.y, area.y, size.y, origin.y)});
}

class BoundedSprite : public MovingSprite {
public:
    using MovingSprite::MovingSprite;

    void setup() override {
        MovingSprite::setup();
        sprite().setSize({160.F, 160.F});
        sprite().setOrigin({0.5F, 0.5F});
        const auto size = m_window -> getMetrics().logicalSize;
        sprite().setPosition({size.x * 0.5F, size.y * 0.5F});
    }

    void update(float deltaTime) override {
        const auto metrics = m_window -> getMetrics();
        if(!metrics.drawable()) return;
        moveInside(deltaTime, {static_cast<float>(metrics.logicalSize.x),
                              static_cast<float>(metrics.logicalSize.y)});
    }

protected:
    void moveInside(float deltaTime, robot2D::vec2f area) {
        MovingSprite::update(deltaTime);
        keepInside(sprite(), area);
    }
};
