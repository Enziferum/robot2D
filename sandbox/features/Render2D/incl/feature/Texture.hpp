//
// Created by Necromant on 15.09.2021.
//

#pragma once

#include <string>

namespace robot2D {
    class Texture2D {
    public:
        Texture2D();
        ~Texture2D();

        bool create();
        bool loadFrom(const std::string& path);
    };
}
