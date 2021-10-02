//
// Created by Necromant on 02.10.2021.
//

#include <robot2D/Graphics/GL.hpp>
#include "OpenGLBuffer.hpp"

namespace robot2D {

    ////// Vertex Buffer //////

    OpenGLVertexBuffer::OpenGLVertexBuffer(const uint32_t& size) {
        glGenBuffers(1, &m_bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        m_size = size;
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(float* data, const uint32_t& size) {
        glGenBuffers(1, &m_bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        m_size = size;
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer() {
        glDeleteBuffers(1, &m_bufferID);
    }

    void OpenGLVertexBuffer::Bind() {
        glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
    }

    void OpenGLVertexBuffer::unBind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void OpenGLVertexBuffer::setData(const void* data, const uint32_t& size) {
        Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    }


    ////// Index Buffer //////

    OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* data, const uint32_t& size)  {
        glGenBuffers(1, &m_bufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer() {
        glDeleteBuffers(1, &m_bufferID);
    }

    void OpenGLIndexBuffer::Bind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
    }

    void OpenGLIndexBuffer::unBind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}