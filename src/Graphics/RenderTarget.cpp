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

#include <ext/glad.h>

#include "robot2D/Graphics/RenderTarget.h"

namespace robot2D {
    const char* vertexShaderSource = "#version 330 core\n"
                                           "layout (location = 0) in vec4 vertex;\n"
                                           "out vec2 TexCoords; \n"
                                           "uniform mat4 projection; \n"
                                           "uniform mat4 model; \n"
                                           "void main()\n"
                                           "{\n"
                                           "   TexCoords = vertex.zw; \n"
                                           "   gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0); \n"
                                           "}\0";

    const char* fragmentShaderSource = "#version 330 core \n"
                                             "in vec2 TexCoords; \n"
                                             "out vec4 color; \n"
                                             "uniform sampler2D sprite;\n"
                                             "uniform vec3 spriteColor;\n"
                                             "void main()\n"
                                             "{\n"
                                             "  color = vec4(spriteColor, 1.0) * texture(sprite, TexCoords);\n"
                                             "}\0";

    void RenderTarget::ortho_projection(matrix& m, float l, float r, float b,
                  float t, float n, float f){
        m.mat[0][0] = 2 / (r - l);

        m.mat[0][1] = 0;
        m.mat[0][2] = 0;
        m.mat[0][3] = 0;

        m.mat[1][0] = 0;
        m.mat[1][1] = 2 / (t - b);
        m.mat[1][2] = 0;
        m.mat[1][3] = 0;

        m.mat[2][0] = 0;
        m.mat[2][1] = 0;
        m.mat[2][2] = -2 / (f - n);
        m.mat[2][3] = 0;

        m.mat[3][0] = -(r + l) / (r - l);
        m.mat[3][1] = -(t + b) / (t - b);
        m.mat[3][2] = -(f + n) / (f - n);
        m.mat[3][3] = 1;
    }


    RenderTarget::RenderTarget(const vec2u& size): m_size(size) {
        setup_GL();
    }

    RenderTarget::~RenderTarget() {}


    void RenderTarget::draw(const Drawable& drawable, const RenderStates& states) {
        drawable.draw(*this, states);
    }

    void RenderTarget::setup_GL() {
        unsigned int VBO;

        float vertices[] = {
                // pos      // tex
                0.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,

                0.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, 0.0f, 1.0f, 0.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                     vertices, GL_STATIC_DRAW);

        glBindVertexArray(VAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                              4 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        if(!m_spriteShaders.createShader(shaderType::vertex,
                                         vertexShaderSource, false))
            return;
        if(!m_spriteShaders.createShader(shaderType::fragment,
                                         fragmentShaderSource, false))
            return;


        m_spriteShaders.use();
        m_spriteShaders.set_parameter("sprite", 0);


        ortho_projection(mat, 0.0f,static_cast<float>(m_size.x),
                 static_cast<float>(m_size.y),
                 0.0f, -1.0f, 1.0f);

        m_spriteShaders.set_parameter("projection", &mat.mat[0][0]);

        //todo use as RenderStates
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void RenderTarget::draw(const RenderStates& states) {
        if(states.shader) {
            // it can be done not here, you can get projection from renderTarget getView !
            // todo remove it in future
            states.shader->set_parameter("projection", &mat.mat[0][0]);
        }
        else {
            m_spriteShaders.use();
            auto &color = states.color;

            float r = color.r / 255.f;
            float g = color.g / 255.f;
            float b = color.b / 255.f;

            m_spriteShaders.set_parameter("spriteColor", r, g, b);

            const robot2D::Transform &transform = states.transform;
            m_spriteShaders.set_parameter("model", transform.get_matrix());
        }

        if(states.texture) {
            glActiveTexture(GL_TEXTURE0);
            states.texture->bind();
        }
        if(states.customVao)
            glBindVertexArray(*states.customVao);
        else
            glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    const matrix &RenderTarget::projection_matrix() const {
        return mat;
    }

}