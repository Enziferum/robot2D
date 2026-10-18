// Core lesson 04: four 64x64 cells, left to right. Zlib license.
#pragma once

#include <cmath>
#include <robot2D/Graphics/Rect.hpp>

class AtlasAnimation {
public:
    void advance(float deltaTime) {
        if(!std::isfinite(deltaTime) || deltaTime <= 0.F) return;
        // Reduce first: a long pause cannot overflow a frame counter or require a long loop.
        m_elapsed = std::fmod(m_elapsed + std::fmod(static_cast<double>(deltaTime), 0.5), 0.5);
    }

    unsigned frame() const { return static_cast<unsigned>(m_elapsed / 0.125); }
    robot2D::IntRect rectangle() const {
        return {static_cast<int>(frame()) * 64, 0, 64, 64};
    }

private:
    double m_elapsed = 0.;
};
