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
#include "OpenGLVertexArray.hpp"

namespace robot2D {

    static GLenum ElementTypeToOpenGL(const ElementType& elementType) {
        switch (elementType) {
            case ElementType::Mat3:
                return GL_FLOAT;
            case ElementType::Mat4:
                return GL_FLOAT;
            case ElementType::Float1:
                return GL_FLOAT;
            case ElementType::Float2:
                return GL_FLOAT;
            case ElementType::Float3:
                return GL_FLOAT;
            case ElementType::Float4:
                return GL_FLOAT;
            case ElementType::Int1:
                return GL_INT;
            case ElementType::Int2:
                return GL_INT;
            case ElementType::Int3:
                return GL_INT;
            case ElementType::Int4:
                return GL_INT;
            case ElementType::Bool:
                return GL_BOOL;
        }
        return GL_FLOAT;
    }

    OpenGLVertexArray::OpenGLVertexArray(): m_vertexIndex(0) {
        glGenVertexArrays(1, &m_VAO);
    }

    OpenGLVertexArray::~OpenGLVertexArray() {
        glDeleteVertexArrays(1, &m_VAO);
    }

    void OpenGLVertexArray::Bind() {
        glBindVertexArray(m_VAO);
    }

    void OpenGLVertexArray::unBind() {
        glBindVertexArray(0);
    }

    void OpenGLVertexArray::setVertexBuffer(const VertexBuffer::Ptr& vertexBuffer) {
        Bind();
        vertexBuffer -> Bind();

        const auto& layout = vertexBuffer -> getAttributeLayout();

        for(const auto& it: layout) {
            switch (it.type) {
                case ElementType::Float1:
                case ElementType::Float2:
                case ElementType::Float3:
                case ElementType::Float4: {
                    glEnableVertexAttribArray(m_vertexIndex);
                    glVertexAttribPointer(m_vertexIndex,
                                          it.getComponentCount(),
                                          ElementTypeToOpenGL(it.type),
                                          it.normalized ? GL_TRUE : GL_FALSE,
                                          layout.getStride(),
                                          (const void *) it.offset);
                    m_vertexIndex++;
                    break;
                }
                case ElementType::Int1:
                case ElementType::Int2:
                case ElementType::Int3:
                case ElementType::Int4:
                case ElementType::Bool:
                {
                    glEnableVertexAttribArray(m_vertexIndex);
                    glVertexAttribIPointer(m_vertexIndex,
                                           it.getComponentCount(),
                                           ElementTypeToOpenGL(it.type),
                                           layout.getStride(),
                                           (const void *) it.offset);
                    m_vertexIndex++;
                    break;
                }
                case ElementType::Mat3:
                case ElementType::Mat4:
                {
                    glEnableVertexAttribArray(m_vertexIndex);
                    auto count = it.getComponentCount();
                    for(unsigned i = 0; i < count; ++i) {
                        glVertexAttribPointer(m_vertexIndex,
                                              count,
                                              ElementTypeToOpenGL(it.type),
                                              it.normalized ? GL_TRUE : GL_FALSE,
                                              layout.getStride(),
                                              (const void*)(it.offset + sizeof(float) * count * i));
                        glVertexAttribDivisor(m_vertexIndex, 1);
                        m_vertexIndex++;
                    }
                    break;
                }
            }
        }

        m_vertexBuffer = vertexBuffer;
    }

    void OpenGLVertexArray::setIndexBuffer(const IndexBuffer::Ptr& indexBuffer) {
        Bind();
        indexBuffer -> Bind();
        m_indexBuffer = indexBuffer;
    }

    const VertexBuffer::Ptr OpenGLVertexArray::getVertexBuffer() const {
        return robot2D::VertexBuffer::Ptr();
    }

    const IndexBuffer::Ptr OpenGLVertexArray::getIndexBuffer() const {
        return robot2D::IndexBuffer::Ptr();
    }
}