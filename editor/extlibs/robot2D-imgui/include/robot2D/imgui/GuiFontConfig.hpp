#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <imgui/imgui.h>

namespace robot2D {
    using FontId = std::uint32_t;

    /// Owning source. Size is logical pixels; merge targets must precede this entry.
    struct GuiFontConfig {
        FontId id = 0;
        FontId mergeInto = 0;
        std::string path;
        std::vector<unsigned char> bytes;
        std::vector<ImWchar> glyphRanges;
        float size = 18.f;
        bool required = true;
    };
}
