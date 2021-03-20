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

#include "Drawable.h"
#include "Font.h"
#include "Color.h"

namespace robot2D{

    class Text: public Drawable{
    public:
        Text();
        ~Text() override = default;

        void setText(const std::string& size);
        void setPos(const robot2D::vec2f& pos);
        robot2D::vec2f& getPos();
        void setScale(const float& scale);
        float& getScale();

        void setCharacterSize(const unsigned int& size);
        void setFont(const Font& font);
    protected:
        void draw(RenderTarget &target, RenderStates states) const override;

    private:
        void setup_GL();
        void update_geometry() const;
    private:
        struct vertex{
            float x;
            float y;
            float tex_x;
            float tex_y;
        };

        struct renderBuffer{
            unsigned int texture;
            std::vector<vertex> verts;
        };

        mutable std::vector<renderBuffer> m_buffer;

        unsigned int VAO;
        unsigned int VBO;
        unsigned int m_chars_size;
        mutable bool m_needupdate;
        std::string m_text;

        mutable robot2D::vec2f m_pos;
        float m_scale;

        Color m_color;
        const Font* m_font;

        ShaderHandler m_textShader;
    };
}