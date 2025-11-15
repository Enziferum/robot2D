#pragma once
#include <string_view>
#include <vector>

namespace robot2D {

    struct GuiFontConfig {
        enum class Mode {
            File,
            Memory
        };

        bool isDefault{ false };
        bool mergeFont{ false };
        float size{ 10.f };
        Mode mode{ Mode::File };
        std::string_view path;
        std::vector<unsigned char>* buffer{ nullptr };
    };

}