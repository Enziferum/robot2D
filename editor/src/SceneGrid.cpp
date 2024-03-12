/*********************************************************************
(c) Alex Raag 2023
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
#include <robot2D/Util/Logger.hpp>

#include <editor/SceneGrid.hpp>

namespace editor {

    const std::string gridVertexShader = R"(
        #version 410 core
        layout (location = 0) in vec2 Position;
        layout (location = 1) in vec2 TexCoord;

        //uniform mat4 projectionMatrix;

        vec3 gridPlane[6] = vec3[] (
            vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
            vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
        );

        void main() {
            vec3 p = vec3(Position.xy, 0.0);
            gl_Position = vec4(p, 1.0); // using directly the clipped coordinates
        }
    )";

    void SceneGrid::setGridElementSize(const size_t& size) {
        m_shader.use();
        m_shader.unUse();
    }

    void SceneGrid::setGridColors(const robot2D::Color &one, const robot2D::Color &two) {
        m_shader.use();
        m_shader.set("colorOne", one);
        m_shader.set("colorTwo", two);
        m_shader.unUse();
    }

    robot2D::vec2f quadVertexPositions[4];

    bool SceneGrid::setup() {
        if(!m_shader.createShader(robot2D::ShaderType::Vertex, gridVertexShader, false)){
            RB_ERROR("Can't setup Grid's vertex shader");
            return false;
        }

        if(!m_shader.createShader(robot2D::ShaderType::Fragment, "res/shaders/gridShader.frag", true)){
            RB_ERROR("Can't setup Grid's fragment shader");
            return false;
        }

        m_vertexArray = robot2D::VertexArray::Create();
        if(!m_vertexArray)
            return false;

        auto vertexBuffer = robot2D::VertexBuffer::Create(sizeof(GridVertex) * m_vertices.size());

        m_vertices[0].texCoord = {0, 0};
        m_vertices[1].texCoord = {1, 0};
        m_vertices[2].texCoord = {1, 1};
        m_vertices[3].texCoord = {0, 1};

        vertexBuffer -> setAttributeLayout( {
            { robot2D::ElementType::Float2, "Position"},
            { robot2D::ElementType::Float2, "TexCoord"}
        });

        uint32_t quadIndices[6];
        quadIndices[0] = 0;
        quadIndices[1] = 1;
        quadIndices[2] = 2;
        quadIndices[3] = 2;
        quadIndices[4] = 3;
        quadIndices[5] = 0;

        m_vertices[0].pos = robot2D::vec2f {-1.F, -1.F};
        m_vertices[1].pos = robot2D::vec2f {1.F, -1.F};
        m_vertices[2].pos = robot2D::vec2f {1.F, 1.F};
        m_vertices[3].pos = robot2D::vec2f {-1.F, 1.F};

        quadVertexPositions[0] = {0.F, 0.F};
        quadVertexPositions[1] = {1.F, 0.F};
        quadVertexPositions[2] = {1.F, 1.F};
        quadVertexPositions[3] = {0.F, 1.F};

        vertexBuffer -> setData(m_vertices.data(), sizeof(GridVertex) * m_vertices.size());
        auto indexBuffer = robot2D::IndexBuffer::Create(quadIndices, 6 * sizeof(uint32_t));
        m_vertexArray -> setVertexBuffer(vertexBuffer);
        m_vertexArray -> setIndexBuffer(indexBuffer);

        return true;
    }

    void SceneGrid::render(robot2D::vec2i size) const{
        m_shader.use();
        m_shader.set("u_resolution", robot2D::vec2f{static_cast<float>(size.x), static_cast<float>(size.y)});

        m_vertexArray -> getVertexBuffer() -> setData(m_vertices.data(), sizeof(GridVertex) * m_vertices.size());
        m_vertexArray -> Bind();

        static auto indicesSize = m_vertexArray -> getIndexBuffer() -> getSize() / 4;
        glDrawElements(GL_TRIANGLES, static_cast<int>(indicesSize), GL_UNSIGNED_INT, nullptr);
        m_vertexArray -> unBind();
        m_shader.unUse();
    }

} // namespace viewer