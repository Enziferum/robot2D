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
    /// Image color channels
    enum ImageColorFormat {
        RED = 1,
        RGB = 3,
        RGBA = 4
    };

    enum class ImageParameter {
        Repeat = 0,
        ClampToEdge
    };

    /**
     * \brief Stores Pixel Buffer in independ format.
     * \details Under robot2D::Texture using Image.
     * Also better to use Image when you need store / modificate Pixel Buffer and don't need Graphics API wrapper.
     */
    class ROBOT2D_EXPORT_API Image {
    public:
        /// \brief Not create pixel buffer only initialize.
        /// \details To fill pixel buffer either use create method to create from own pixel buffer or
        /// load image from disk.
        Image();
        ~Image() = default;

        /**
         * \brief load image from disk.
         * @param path absolute or relative path to file on disk.
         * @return Were loading process is success?
         */
        bool loadFromFile(const std::string& path, int desiredChannels = 0);

        /// Full size.
        const robot2D::vec2u& getSize() const;

        /// Full size.
        robot2D::vec2u& getSize();

        /// Image has several variant of channels.
        const ImageColorFormat& getColorFormat() const;

        const ImageParameter& getImageParameter() const;

        /// Returns pixelBuffer which not possible to modificate.
        const std::vector<std::uint8_t>& getBuffer() const;

        /// Returns pixelBuffer which possible to modificate.
        std::uint8_t* getBuffer();

        /// Creating image from PixelBuffer and specify Color channels directly.
        bool create(const vec2u& size, const void* data, const ImageColorFormat&,
                    const ImageParameter& parameter = ImageParameter::Repeat);

        /// Save onto disk by absolute or relative path.
        bool save(const std::string& path);
    private:
        vec2u m_size;
        std::vector<std::uint8_t> m_pixels;
        ImageColorFormat m_colorFormat = ImageColorFormat::RED;
        ImageParameter m_imageParameter;
    };

}
