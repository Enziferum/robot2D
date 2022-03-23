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

#include <robot2D/Graphics/Buffer.hpp>

#include "OpenGL/OpenGLBuffer.hpp"

namespace robot2D {
    ////// Vertex Buffer //////

    VertexBuffer::~VertexBuffer() {}

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

    IndexBuffer::~IndexBuffer() {}

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