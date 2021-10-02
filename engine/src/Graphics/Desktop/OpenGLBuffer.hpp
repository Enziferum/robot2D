//
// Created by Necromant on 02.10.2021.
//

#pragma once

#include <robot2D/Graphics/Buffer.hpp>

namespace robot2D {
    class OpenGLVertexBuffer final:  public VertexBuffer {
    public:
        OpenGLVertexBuffer(const uint32_t& size);
        OpenGLVertexBuffer(float* data, const uint32_t& size);
        virtual ~OpenGLVertexBuffer() override;

        void Bind() override;
        void unBind() override;

        void setData(const void* data, const uint32_t &size) override;
        void setAttributeLayout(const AttributeLayout& layout) { m_layout = layout;}
        const AttributeLayout& getAttributeLayout() const { return m_layout;}
    private:
        unsigned m_bufferID;
        AttributeLayout m_layout;
    };

    class OpenGLIndexBuffer final: public IndexBuffer {
    public:
        OpenGLIndexBuffer(uint32_t* data, const uint32_t& size);
        ~OpenGLIndexBuffer();

        void Bind() override;
        void unBind() override;
    private:
        unsigned m_bufferID;
    };
}
