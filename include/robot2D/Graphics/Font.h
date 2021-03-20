/*********************************************************************
(c) Alex Raag 2021
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
#include <map>

#include "robot2D/Core/Vector2.h"


namespace robot2D{

    struct character {
        unsigned int textureID; // ID handle of the glyph texture
        vec2i   size;      // size of glyph
        vec2i   bearing;   // offset from baseline to left/top of glyph
        unsigned int advance;   // horizontal offset to advance to next glyph
    };


    class Font{
    public:
        Font();
        ~Font();

        bool loadFromFile(const std::string& path, const unsigned int& char_size = 20);
        const std::map<char, character>& get_chars()const;
    private:
        void cleanup();
        void setup_cache();
        void generate_texture(unsigned int& texture, void* glyph);
    private:
        void* m_library;
        void* m_face;
        std::map<char, character> m_characters;
    };
}