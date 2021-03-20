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

#include <iostream>
#include <glad/glad.h>
#include "stb_image/stb_image.h"

#include "robot2D/Graphics/Texture.h"

namespace robot2D{

    Texture::Texture():m_size(),
    buffer(nullptr)
    {
        setup_GL();
    }

    Texture::~Texture() {
        if(buffer)
            stbi_image_free(buffer);
    }

    bool Texture::loadFromFile(const std::string& path, bool alpha) {
        int width, height, nrChannels;
        buffer = stbi_load(path.c_str(), &width, &height,
                                        &nrChannels, 0);

        m_size.x = width;
        m_size.y = height;

        if(!buffer)
            return false;

        GLenum format;

        format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        if (nrChannels == 3)
            format = GL_RGB;
        if (nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, m_size.x, m_size.y,
                     0, format, GL_UNSIGNED_BYTE, buffer);
        glGenerateMipmap(GL_TEXTURE_2D);

        return true;
    }

    vec2u& Texture::get_size() {
        return m_size;
    }

    const vec2u &Texture::get_size() const {
        return m_size;
    }

    void Texture::setup_GL() {
        glGenTextures(1, &m_texture);
        // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
        glBindTexture(GL_TEXTURE_2D, m_texture);
        // set the texture wrapping parameters
        // set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void Texture::bind() const {
        glBindTexture(GL_TEXTURE_2D, m_texture);
    }

    void Texture::generate(const vec2u& size, void *data) {
        m_size = size;

        // create Texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }

    const unsigned int& Texture::get_id() const {
        return m_texture;
    }


}
