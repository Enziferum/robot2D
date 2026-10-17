#pragma once

#include "../bounded-sprite/BoundedSprite.hpp"
#include "AtlasAnimation.hpp"
#include "BeaconAtlas.hpp"

class AnimatedSprite : public BoundedSprite {
public:
    using BoundedSprite::BoundedSprite;

    void setup() override {
        // Upload only after Engine has created the GL context.
        const auto pixels = makeBeaconAtlas();
        m_atlas.create({256, 64}, pixels.data());
        sprite().setTexture(m_atlas, m_animation.rectangle());
        sprite().setSize({160.F, 160.F});
        sprite().setOrigin({0.5F, 0.5F});
        const auto size = m_window -> getMetrics().logicalSize;
        sprite().setPosition({size.x * 0.5F, size.y * 0.5F});
        m_running = true;
    }

    void update(float deltaTime) override {
        BoundedSprite::update(deltaTime);
        advanceAnimation(deltaTime);
    }

    void destroy() override {
        m_atlas.release();
        FirstSprite::destroy();
    }

protected:
    void advanceAnimation(float deltaTime) {
        m_animation.advance(deltaTime);
        sprite().setTextureRect(m_animation.rectangle());
    }

private:
    robot2D::Texture m_atlas;
    AtlasAnimation m_animation;
};
