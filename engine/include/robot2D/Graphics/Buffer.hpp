//
// Created by Necromant on 02.10.2021.
//

#pragma once

#include <memory>
#include <cstdint>
#include <string>
#include <vector>

namespace robot2D {

    enum class ElementType {
        Mat3, Mat4, Float1, Float2, Float3, Float4, Int1, Int2, Int3, Int4, Bool
    };

    static uint32_t ElementTypeSize(const ElementType& elementType) {
        switch (elementType) {
            case ElementType::Mat3:
                return 4 * 4 * 3;
            case ElementType::Mat4:
                return 4 * 4 * 4;
            case ElementType::Float1:
                return 4;
            case ElementType::Float2:
                return 4 * 2;
            case ElementType::Float3:
                return 4 * 3;
            case ElementType::Float4:
                return 4 * 4;
            case ElementType::Int1:
                return 4;
            case ElementType::Int2:
                return 4 * 2;
            case ElementType::Int3:
                return 4 * 3;
            case ElementType::Int4:
                return 4 * 4;
            case ElementType::Bool:
                return 1;
        }
    }

    struct Element {
        Element(const ElementType& elementType, const std::string& name, bool normalized = false):
        type(elementType),
        name(name),
        size(ElementTypeSize(elementType)),
        offset(0), normalized(normalized) {}

        ~Element() = default;

        uint32_t getComponentCount() const {
            switch (type) {
                case ElementType::Mat3:
                    return 3;
                case ElementType::Mat4:
                    return 4;
                case ElementType::Float1:
                    return 1;
                case ElementType::Float2:
                    return 2;
                case ElementType::Float3:
                    return 3;
                case ElementType::Float4:
                    return 4;
                case ElementType::Int1:
                    return 1;
                case ElementType::Int2:
                    return 2;
                case ElementType::Int3:
                    return 3;
                case ElementType::Int4:
                    return 4;
                case ElementType::Bool:
                    return 1;
            }
        }

        std::string name;
        ElementType type;
        uint32_t size;
        size_t offset;
        bool normalized;
    };

    struct AttributeLayout {
    public:
        AttributeLayout() = default;
        AttributeLayout(std::initializer_list<Element> elements): m_elements(elements) {
            calculateStrideAndOffset();
        }
        ~AttributeLayout() = default;

        const uint32_t& getStride() const {
            return m_stride;
        }

        std::vector<Element>::iterator begin() { return m_elements.begin(); }
        std::vector<Element>::iterator end() { return m_elements.end(); }
        std::vector<Element>::const_iterator begin() const{ return m_elements.begin(); }
        std::vector<Element>::const_iterator end() const { return m_elements.end(); }
    private:
        void calculateStrideAndOffset() {
            size_t offset = 0;
            m_stride = 0;
            for(auto& it: m_elements) {
                it.offset = offset;
                offset += it.size;
                m_stride += it.size;
            }
        }

        std::vector<Element> m_elements;
        uint32_t m_stride = 0;
    };

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
        virtual void setAttributeLayout(const AttributeLayout& layout) = 0;
        virtual const AttributeLayout& getAttributeLayout() const = 0;

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
