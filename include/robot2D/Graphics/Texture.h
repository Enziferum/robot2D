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
#include "robot2D/Core/Vector2.h"

namespace robot2D{
    class Texture{
    public:
        Texture();
        ~Texture();

        bool loadFromFile(const std::string& path, bool alpha = false);

        vec2u& get_size();
        const vec2u& get_size() const;

        void generate(const vec2u& size, void* data);
        const unsigned int& get_id()const;
        void bind()const;
    private:
        void setup_GL();
    private:
        vec2u m_size;
        unsigned int m_texture;
        unsigned char* buffer;
    };
}