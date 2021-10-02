//
// Created by Necromant on 02.10.2021.
//

#pragma once

#include <memory>
#include "Buffer.hpp"

namespace robot2D {
    class VertexArray {
    public:
        using Ptr = std::shared_ptr<VertexArray>;
    public:
        virtual ~VertexArray() = default;

        virtual void Bind() = 0;
        virtual void unBind() = 0;
        virtual void setVertexBuffer(const VertexBuffer::Ptr& vertexBuffer) = 0;
        virtual void setIndexBuffer(const IndexBuffer::Ptr& indexBuffer) = 0;
        virtual const VertexBuffer::Ptr getVertexBuffer() const = 0;
        virtual const IndexBuffer::Ptr getIndexBuffer() const = 0;

        static Ptr Create();
    };
}
