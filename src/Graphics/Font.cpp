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

#include "ft2build.h"
#include FT_FREETYPE_H

#include "robot2D/Graphics/Font.h"

namespace robot2D{
    const short chars_cache = 128;

    Font::Font():
        m_library(nullptr),
        m_face(nullptr){
    }

    Font::~Font() {
        cleanup();
    }

    bool Font::loadFromFile(const std::string& path, const unsigned int& char_size) {
        cleanup();

        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {// all functions return a value different than 0 whenever an error occurred
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return false;
        }

        m_library = ft;

        // load font as face
        FT_Face face;
        if (FT_New_Face(static_cast<FT_Library>(m_library), path.c_str(), 0, &face)) {
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
            return false;
        }

        m_face = face;

        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, char_size);
        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // then for the first 128 ASCII characters, pre-load/compile their characters and store them
        setup_cache();

        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }

    void Font::cleanup() {
        if(m_library)
            FT_Done_FreeType(static_cast<FT_Library>(m_library));
        if(m_face)
            FT_Done_Face(static_cast<FT_Face>(m_face));

        m_library = nullptr;
        m_face = nullptr;
    }

    void Font::setup_cache() {
        FT_Face face = static_cast<FT_Face>(m_face);

        for (short c = 0; c < chars_cache; ++c) // lol see what I did there
         {
             if (FT_Load_Char(face, c, FT_LOAD_RENDER))
             {
                 std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                 continue;
             }
             // generate texture
             unsigned int texture;
             generate_texture(texture, face->glyph);
             // load character glyph

            // now store character for later use
            character charact = {
                    texture,
                    vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                    vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    (unsigned int)(face->glyph->advance.x)
            };

            m_characters.insert(std::pair<char, character>(c, charact));
        }

    }

    void Font::generate_texture(unsigned int& texture, void* fglyph) {
        FT_GlyphSlot glyph = static_cast<FT_GlyphSlot>(fglyph);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                glyph->bitmap.width,
                glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    const std::map<char, character> &Font::get_chars() const {
        return m_characters;
    }

}