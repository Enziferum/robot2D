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
#include "OpenGLBuffer.hpp"

namespace robot2D {

    ////// Vertex Buffer //////

    OpenGLVertexBuffer::OpenGLVertexBuffer(const uint32_t& size) {

        glCall(glGenBuffers, 1, &m_bufferID);
        glCall(glBindBuffer, GL_ARRAY_BUFFER, m_bufferID);
        glCall(glBufferData, GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

        m_size = size;
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(float* data, const uint32_t& size) {
        glCall(glGenBuffers, 1, &m_bufferID);
        glCall(glBindBuffer, GL_ARRAY_BUFFER, m_bufferID);
        glCall(glBufferData, GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        m_size = size;
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer() {
        glCall(glDeleteBuffers, 1, &m_bufferID);
    }

    void OpenGLVertexBuffer::Bind() {
        glCall(glBindBuffer, GL_ARRAY_BUFFER, m_bufferID);
    }

    void OpenGLVertexBuffer::unBind() {
        glCall(glBindBuffer, GL_ARRAY_BUFFER, 0);
    }

    void OpenGLVertexBuffer::setData(const void* data, const uint32_t& size) {
        Bind();
        glCall(glBufferSubData, GL_ARRAY_BUFFER, 0, size, data);
    }


    ////// Index Buffer //////

    OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* data, const uint32_t& size)  {
        glCall(glGenBuffers, 1, &m_bufferID);
        glCall(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
        glCall(glBufferData, GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        m_size = size;
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer() {
        glCall(glDeleteBuffers, 1, &m_bufferID);
    }

    void OpenGLIndexBuffer::Bind() {
        glCall(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
    }

    void OpenGLIndexBuffer::unBind() {
        glCall(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}