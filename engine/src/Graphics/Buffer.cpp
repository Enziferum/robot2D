//
// Created by Necromant on 02.10.2021.
//

#include <robot2D/Graphics/Buffer.hpp>

#include "Desktop/OpenGLBuffer.hpp"

namespace robot2D {
    ////// Vertex Buffer //////

    VertexBuffer::~VertexBuffer() noexcept {}

    const uint32_t& VertexBuffer::getSize() const {
        return m_size;
    }

    uint32_t& VertexBuffer::getSize() {
        return m_size;
    }

    VertexBuffer::Ptr VertexBuffer::Create(const uint32_t& size) {
        return std::make_shared<OpenGLVertexBuffer>(size);
    }

    VertexBuffer::Ptr VertexBuffer::Create(float* data, const uint32_t &size) {
        return std::make_shared<OpenGLVertexBuffer>(data, size);
    }

    ////// Index Buffer //////

    IndexBuffer::~IndexBuffer() noexcept {}

    const uint32_t& IndexBuffer::getSize() const {
        return m_size;
    }

    uint32_t& IndexBuffer::getSize() {
        return m_size;
    }

    IndexBuffer::Ptr IndexBuffer::Create(uint32_t* data, const uint32_t& size) {
        return std::make_shared<OpenGLIndexBuffer>(data, size);
    }
}