/*********************************************************************
(c) Alex Raag 2024
https://github.com/Enziferum
robot2D - Zlib license.
This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#pragma once

#include <string>
#include <unordered_map>
#include "Animation.hpp"

namespace editor {

    class AnimationParser {
    public:
        AnimationParser();
        AnimationParser(const AnimationParser& other) = delete;
        AnimationParser& operator=(const AnimationParser& other) = delete;
        AnimationParser(AnimationParser&& other) = delete;
        AnimationParser& operator=(AnimationParser&& other) = delete;
        ~AnimationParser() = default;

        bool loadFromFile(const std::string& path, Animation* animation);
        bool saveToFile(const std::string& path, const Animation* animation);
    private:
        enum class XmlKey {
            image,
            maskColor,
            head,
            animation,
            cut,
            title,
            delay,
            x_coord,
            y_coord,
            width,
            height
        };
        std::unordered_map<XmlKey, std::string> m_keys;
    };

}