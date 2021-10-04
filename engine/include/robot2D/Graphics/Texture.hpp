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
#include <vector>
#include "robot2D/Core/Vector2.hpp"

namespace robot2D {
    class ROBOT2D_EXPORT_API Texture {
    public:
        enum class ColorFormat {
            Red = 1,
            Rgb = 3,
            Rgba = 4
        };
    public:
        Texture();
        ~Texture();

        bool loadFromFile(const std::string& path, bool usealpha = false);
        unsigned char* getPixels() const;

        vec2u& getSize();
        const vec2u& getSize() const;

        void create(const vec2u& size, void* data, const ColorFormat& colorFormat = ColorFormat::Rgba);
        const unsigned int& getID()const;

        const ColorFormat& getColorFormat() const { return m_colorFormat; }
    private:
        void bindBufferData(void* bufferData);
        void setupGL();
    private:
        vec2u m_size;
        unsigned int m_texture;
        unsigned char* buffer;
        std::vector<unsigned char> m_data;
        ColorFormat m_colorFormat;
    };
}