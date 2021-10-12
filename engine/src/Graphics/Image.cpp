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

#ifdef WIN32
    #include <robot2D/ext/stb_image.h>
#elif __APPLE__
    #include <ext/stb_image.h>
#endif
#include <robot2D/Graphics/Image.hpp>
#include <robot2D/Util/Logger.hpp>

namespace robot2D {

    Image::Image(): m_pixels() {}

    bool Image::loadFromFile(const std::string& path) {
        m_pixels.clear();

        // Load the image and get a pointer to the pixels in memory
        int width = 0;
        int height = 0;
        int channels = 0;
        //STBI_rgb_alpha
        unsigned char* ptr = stbi_load(path.c_str(), &width, &height, &channels, 0);

        if (ptr)
        {
            // Assign the image properties
            m_size.x = width;
            m_size.y = height;

            if (width && height)
            {
                // Copy the loaded pixels to the pixel buffer
                m_pixels.resize(width * height * 4);
                memcpy(&m_pixels[0], ptr, m_pixels.size());
            }

            // Free the loaded pixels (they are now in our own pixel buffer)
            stbi_image_free(ptr);
            m_colorFormat = static_cast<ImageColorFormat>(channels);
            return true;
        }
        else
        {
            // Error, failed to load the image
            RB_CORE_ERROR("Failed to load image {0}. Reason: {1}", path, stbi_failure_reason());
            return false;
        }
        return true;
    }

    const robot2D::vec2u& Image::getSize() const {
        return m_size;
    }

    const ImageColorFormat& Image::getColorFormat() const {
        return m_colorFormat;
    }

    const std::vector<unsigned char>& Image::getBuffer() const {
        return m_pixels;
    }

    robot2D::vec2u &Image::getSize() {
        return m_size;
    }

    unsigned char* Image::getBuffer() {
        if(m_pixels.empty())
            return nullptr;
        return m_pixels.data();
    }

    bool Image::create(const vec2u& size, void *data, const ImageColorFormat& colorFormat) {
        m_colorFormat = colorFormat;
        m_size = size;
        return true;
    }

}

