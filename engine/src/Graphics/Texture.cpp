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

#include <robot2D/Graphics/GL.hpp>
#ifdef WIN32
    #include <robot2D/ext/stb_image.h>
#elif __APPLE__
    #include <ext/stb_image.h>
#endif
#include <robot2D/Graphics/Texture.hpp>

namespace robot2D{

    GLenum convertColorType(const Texture::ColorFormat& format) {
        switch(format) {
            case Texture::ColorFormat::Red:
                return GL_RED;
            case Texture::ColorFormat::Rgb:
                return GL_RGB;
            case Texture::ColorFormat::Rgba:
                return GL_RGBA;
        }
    }

    Texture::Texture():
    m_size(),
    buffer(nullptr),
    m_data() {}

    Texture::~Texture() {
        if(buffer)
            stbi_image_free(buffer);
    }

    bool Texture::loadFromFile(const std::string& path, bool usealpha) {
        int width, height, nrChannels;
        buffer = stbi_load(path.c_str(), &width, &height,
                                        &nrChannels, 0);

        if(buffer == nullptr)
            return false;

        m_data.resize(width * height * 4);
        m_size.x = width;
        m_size.y = height;
        m_colorFormat = static_cast<ColorFormat>(nrChannels);
        if(usealpha)
            m_colorFormat = ColorFormat::Rgba;
        setupGL();
        bindBufferData(buffer);
        glGenerateMipmap(GL_TEXTURE_2D);

        return true;
    }

    vec2u& Texture::getSize() {
        return m_size;
    }

    const vec2u& Texture::getSize() const {
        return m_size;
    }

    void Texture::setupGL() {
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void Texture::create(const vec2u& size, void* data, const ColorFormat& colorFormat) {
        m_size = size;
        m_colorFormat = colorFormat;
        setupGL();
        bindBufferData(data);
    }

    const unsigned int& Texture::getID() const {
        return m_texture;
    }

    unsigned char* Texture::getPixels() const {
        return buffer;
    }

    void Texture::bindBufferData(void* bufferData) {
        auto glFormat = convertColorType(m_colorFormat);
        glTexImage2D(GL_TEXTURE_2D, 0, glFormat, m_size.x, m_size.y, 0, glFormat, GL_UNSIGNED_BYTE, bufferData);
    }

}
