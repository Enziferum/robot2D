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

#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/Text.hpp>
#include <robot2D/Graphics/RenderTarget.hpp>

namespace robot2D {
    constexpr int buffer_sz = 24;

    const char* vertexText = "#version 330 core\n"
                                     "layout (location = 0) in vec4 vertex;\n"
                                     "out vec2 TexCoords; \n"
                                     "uniform mat4 projection; \n"
                                     "void main()\n"
                                     "{\n"
                                     "   TexCoords = vertex.zw; \n"
                                     "   gl_Position = projection * vec4(vertex.xy, 0.0, 1.0); \n"
                                     "}\0";

    const char* fragmentText = "#version 330 core \n"
                                       "in vec2 TexCoords; \n"
                                       "out vec4 color; \n"
                                       "uniform sampler2D text;\n"
                                       "uniform vec3 textColor;\n"
                                       "void main()\n"
                                       "{\n"
                                       "  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
                                       "  color = vec4(textColor, 1.0) * sampled;\n"
                                       "}\0";

    Text::Text():
        m_buffer(),
        m_needupdate(false),
        m_text(""),
        m_pos(),
        m_scale(1.f),
        m_color(Color::White)
        {
          //  setup_GL();
        }

    void Text::setText(const std::string& text) {
        m_text = text;
        m_needupdate = true;
    }

    void Text::setCharacterSize(const unsigned int &size) {
        if(size == 0)
            return;
    }

    void Text::setup_GL() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buffer_sz, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        m_textShader.createShader(shaderType::vertex, vertexText, false);
        m_textShader.createShader(shaderType::fragment, fragmentText, false);

        m_textShader.use();
        m_textShader.set_parameter("text", 0);
    }

    void Text::draw(RenderTarget& target, RenderStates states) const {
        return;
        (void)target;
        if(!m_font)
            return;

        update_geometry();

       // glActiveTexture(GL_TEXTURE0);
        m_textShader.use();
        m_textShader.set_parameter("textColor", m_color.red, m_color.green, m_color.blue);

        //draw by char
        for(auto& it: m_buffer) {
            states.color = m_color;
            states.customVao = &VAO;
            states.shader = const_cast<robot2D::ShaderHandler*>(&m_textShader);
            glBindTexture(GL_TEXTURE_2D, it.texture);
            // update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex) * it.verts.size() , &it.verts[0]);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            //target.draw(states);
            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void Text::setFont(const Font& font) {
        m_font = &font;
    }

    void Text::update_geometry() const {
        if(!m_needupdate || !m_font)
            return;

        m_buffer.clear();

        std::string::const_iterator it;
        //for updating
        vec2f copy_pos = m_pos;
        for (it = m_text.begin(); it != m_text.end(); ++it)
        {
            auto m_characters = m_font -> get_chars();
            character ch = m_characters[*it];

            float xpos = copy_pos.x + ch.bearing.x * m_scale;
            float ypos = copy_pos.y + (m_characters['H'].bearing.y - ch.bearing.y) * m_scale;

            float w = ch.size.x * m_scale;
            float h = ch.size.y * m_scale;

            // updated vbo
            std::vector<vertex> vertices = {
                    { xpos,     ypos + h,   0.0f, 1.0f },
                    { xpos + w, ypos,       1.0f, 0.0f },
                    { xpos,     ypos,       0.0f, 0.0f },

                    { xpos,     ypos + h,   0.0f, 1.0f },
                    { xpos + w, ypos + h,   1.0f, 1.0f },
                    { xpos + w, ypos,       1.0f, 0.0f }
            };

            renderBuffer buffer = {
                    ch.textureID,
                    vertices
            };
            m_buffer.push_back(buffer);

            copy_pos.x += (ch.advance >> 6) * m_scale; // bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)
        }

        m_needupdate = false;
    }

    void Text::setPos(const vec2f& pos) {
        m_pos = pos;
    }

    robot2D::vec2f& Text::getPos() {
        return m_pos;
    }

    void Text::setScale(const float& scale) {
        m_scale = scale;
    }

    float &Text::getScale() {
        return m_scale;
    }


}