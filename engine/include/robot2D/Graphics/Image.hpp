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
#include <robot2D/Core/Vector2.hpp>

namespace robot2D {

    enum ImageColorFormat {
        RED = 1,
        LUMINANCE = 2,
        RGB = 3,
        RGBA = 4
    };

    /// Image stores Texture Data as Buffer
    class ROBOT2D_EXPORT_API Image {
    public:
        Image();
        ~Image() = default;

        bool loadFromFile(const std::string& path);

        const robot2D::vec2u& getSize() const;
        robot2D::vec2u& getSize();
        const ImageColorFormat& getColorFormat() const;
        const std::vector<unsigned char>& getBuffer() const;
        unsigned char* getBuffer();

        bool create(const vec2u& size, const void* data, const ImageColorFormat&);
        bool save(const std::string& path);
    private:
        vec2u m_size;
        std::vector<uint8_t> m_pixels;
        ImageColorFormat m_colorFormat = ImageColorFormat::RED;
    };

}
