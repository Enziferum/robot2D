//
// Created by Necromant on 02.10.2021.
//

#pragma once

#include <memory>
#include <cstdint>

namespace robot2D {



    // Public Interface of VertexBuffer //
    class VertexBuffer {
    public:
        using Ptr = std::shared_ptr<VertexBuffer>;
    public:
        virtual ~VertexBuffer();

        virtual void Bind() = 0;
        virtual void unBind() = 0;

        const uint32_t& getSize() const;
        uint32_t& getSize();

        virtual void setData(const void* data, const uint32_t& size) = 0;

        static Ptr Create(const uint32_t& size);
        static Ptr Create(float* data, const uint32_t& size);
    protected:
        uint32_t m_size;
    };

    // Public Interface of IndexBuffer //
    class IndexBuffer {
    public:
        using Ptr = std::shared_ptr<IndexBuffer>;
    public:
        virtual ~IndexBuffer() = 0;

        virtual void Bind() = 0;
        virtual void unBind() = 0;

        const uint32_t& getSize() const;
        uint32_t& getSize();

        static Ptr Create(uint32_t* data, const uint32_t& size);
    protected:
        uint32_t m_size;
    };
}
