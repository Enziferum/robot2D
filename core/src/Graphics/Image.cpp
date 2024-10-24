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

#include "internal/stb_image.h"
#include "internal/stb_image_write.h"

#include <robot2D/Graphics/Image.hpp>
#include <robot2D/Util/Logger.hpp>

namespace robot2D {
    #define STB_IMAGE_WRITE_IMPLEMENTATION

    Image::Image(): m_pixels() {}

    bool Image::loadFromFile(const std::string& path, int desiredChannels) {
        m_pixels.clear();

        int width = 0;
        int height = 0;
        int channels = 0;
        unsigned char* ptr = stbi_load(path.c_str(), &width, &height, &channels, desiredChannels);
        if (ptr)
        {
            // Assign the image properties
            m_size.x = static_cast<unsigned int>(width);
            m_size.y = static_cast<unsigned int>(height);

            if (width > 0 && height > 0)
            {
                // Copy the loaded pixels to the pixel buffer
                m_pixels.resize(static_cast<std::size_t>(width * height * channels));
                memcpy(m_pixels.data(), ptr, m_pixels.size());
            }

            // Free the loaded pixels (they are now in our own pixel buffer)
            stbi_image_free(ptr);
            m_colorFormat = static_cast<ImageColorFormat>(channels);
            return true;
        }
        else
        {
            // Error, failed to load the image
            RB_CORE_ERROR("Failed to load image {0}. Reason: {1}", path, std::string{stbi_failure_reason()});
            return false;
        }
    }

    const robot2D::vec2u& Image::getSize() const {
        return m_size;
    }

    robot2D::vec2u& Image::getSize() {
        return m_size;
    }

    const ImageColorFormat& Image::getColorFormat() const {
        return m_colorFormat;
    }

    const std::vector<unsigned char>& Image::getBuffer() const {
        return m_pixels;
    }

    std::uint8_t* Image::getBuffer() {
        if(m_pixels.empty())
            return nullptr;
        return m_pixels.data();
    }

    bool Image::create(const vec2u& size, const void* data, const ImageColorFormat& colorFormat,
                       const ImageParameter& parameter) {
        m_pixels.clear();
        m_colorFormat = colorFormat;
        m_size = size;
        m_imageParameter = parameter;

        int channels = 1;
        switch(colorFormat) {
            case RED:
                break;
            case RGB:
                channels = 3;
                break;
            case RGBA:
                channels = 4;
                break;
        }
        m_pixels.resize(size.x * size.y * channels);
        if(data == nullptr) {
            RB_CORE_CRITICAL("Can't Create Image in buffer is nullptr");
            return false;
        }
        memcpy(m_pixels.data(), data, m_pixels.size());
        return true;
    }

    bool Image::save(const std::string& path) {
        int channelsNum;
        switch(m_colorFormat) {
            case RED:
                channelsNum = 1;
                break;
            case RGB:
                channelsNum = 3;
                break;
            case RGBA:
                channelsNum = 4;
                break;
        }

        return false;
//        return stbi_write_png(path.c_str(), static_cast<int>(m_size.x), static_cast<int>(m_size.y),
//                              channelsNum, m_pixels.data(), channelsNum * m_size.x);
    }

    const ImageParameter& Image::getImageParameter() const {
        return m_imageParameter;
    }
}

