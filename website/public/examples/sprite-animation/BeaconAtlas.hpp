// A tiny code-generated teaching asset, not a Robot Lab walking animation.
#pragma once

#include <array>

inline std::array<unsigned char, 256 * 64 * 4> makeBeaconAtlas() {
    std::array<unsigned char, 256 * 64 * 4> pixels{};
    for(unsigned frame = 0; frame < 4; ++frame) {
        for(unsigned y = 8; y < 56; ++y) {
            for(unsigned x = 8; x < 56; ++x) {
                const auto i = (y * 256 + frame * 64 + x) * 4;
                const bool light = y >= 20 && y < 36 && x >= 12 + frame * 10 && x < 22 + frame * 10;
                pixels[i] = light ? 124 : 40;
                pixels[i + 1] = light ? 219 : 60;
                pixels[i + 2] = light ? 216 : 74;
                pixels[i + 3] = 255;
            }
        }
    }
    return pixels;
}
