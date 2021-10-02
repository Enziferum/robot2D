//
// Created by Necromant on 02.10.2021.
//

#pragma once

#include <robot2D/Graphics/VertexArray.hpp>

namespace robot2D {

    class OpenGLVertexArray final: public VertexArray {
    public:
        OpenGLVertexArray();
        ~OpenGLVertexArray();

        virtual void Bind() override;
        virtual void unBind() override;
        virtual void setVertexBuffer(const VertexBuffer::Ptr& vertexBuffer) override;
        virtual void setIndexBuffer(const IndexBuffer::Ptr& indexBuffer) override;
        virtual const VertexBuffer::Ptr getVertexBuffer() const override;
        virtual const IndexBuffer::Ptr getIndexBuffer() const override;
    private:
        VertexBuffer::Ptr m_vertexBuffer;
        IndexBuffer::Ptr m_indexBuffer;
        unsigned int m_VAO;
        unsigned int m_vertexIndex;
    };

}
