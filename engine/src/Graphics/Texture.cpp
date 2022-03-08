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
#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Util/Logger.hpp>

namespace robot2D{

    GLenum convertColorType(const ImageColorFormat& format) {
        switch(format) {
            case ImageColorFormat::Red:
                return GL_RED;
            case ImageColorFormat::RGB:
                return GL_RGB;
            case ImageColorFormat::RGBA:
                return GL_RGBA;
        }
    }

    Texture::Texture(){}

    Texture::~Texture() {
        glDeleteTextures(1, &m_texture);
    }

    bool Texture::loadFromFile(const std::string& path) {
        if(!m_image.loadFromFile(path))
            return false;
        setupGL();
        bindBufferData(m_image.getBuffer());
        return true;
    }

    vec2u& Texture::getSize() {
        return m_image.getSize();
    }

    const vec2u& Texture::getSize() const {
        return m_image.getSize();
    }

    void Texture::setupGL() {
#ifdef ROBOT2D_MACOS
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
#elif ROBOT2D_WINDOWS
        glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
#endif

        auto size = m_image.getSize();
        GLint internalFormat = GL_RGB8;
        auto glFormat = convertColorType(m_image.getColorFormat());
        if(glFormat == GL_RGBA)
            internalFormat = GL_RGBA8;
#ifdef ROBOT2D_MACOS
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#elif ROBOT2D_WINDOWS
        glTextureStorage2D(m_texture, 1, internalFormat, size.x, size.y);

        glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif
    }

    void Texture::create(const vec2u& size, void* data, const ImageColorFormat& colorFormat) {
        m_image.create(size, data, colorFormat);
        setupGL();
        bindBufferData(data);
    }

    const unsigned int& Texture::getID() const {
        return m_texture;
    }

    const unsigned char* Texture::getPixels() const {
        return m_image.getBuffer().data();
    }

    unsigned char* Texture::getPixels(){
        return m_image.getBuffer();
    }

    void Texture::bindBufferData(void* bufferData) {
        auto glFormat = convertColorType(m_image.getColorFormat());
        auto size = m_image.getSize();
#ifdef ROBOT2D_MACOS
        glTexImage2D(GL_TEXTURE_2D, 0, glFormat, size.x,
                     size.y, 0, glFormat, GL_UNSIGNED_BYTE, bufferData);
#elif ROBOT2D_WINDOWS
        glTextureSubImage2D(m_texture, 0, 0, 0, size.x, size.y, glFormat, GL_UNSIGNED_BYTE, bufferData);
#endif
    }

    void Texture::bind(uint32_t slot) {
#ifdef ROBOT2D_MACOS
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_texture);
#elif ROBOT2D_WINDOWS
        glBindTextureUnit(slot, m_texture);
#endif
    }

}
