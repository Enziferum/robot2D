#pragma once
#include <cassert>
#include <robot2D/Graphics/VertexArray.hpp>

namespace robot2D {
    namespace {
        int maxQuads = 1000;

        int maxVertexCount = maxQuads * 4;
        int maxIndexCount = maxQuads * 6;
    }

    template<typename BatchVertex>
    struct QuadBatchRender {
        ~QuadBatchRender();

        /// Prepare Vertex Array/Buffer/Index
        void initalize(robot2D::AttributeLayout);

        inline void pack(BatchVertex&& vertex);

        /// Reset BatchElement Buffer
        void preProcessBatching();

        /// Set BatchElement Buffer
        void processBatching();

        /// Reset counters
        void refresh();

        /// When current indexcount > maxValue need direct refresh
        bool needRefresh() const;

        /// Bind internal VertexArray
        void Bind();

        /// UnBind internal VertexArray
        void unBind();

        int getIndexCount() const;

        robot2D::VertexArray::Ptr getVertexArray() const { return m_vertexArray; }
    private:
        const int quadSize = 4;
        const int quadIndicesSize = 6;

        /// Counters ///
        int batchElementIndex = 0;
        int indexCount = 0;
        /// need update indexCount and check valid vertex / indices count
        int cutQuadCntr = 1;
        /// Counters ///

        robot2D::VertexArray::Ptr m_vertexArray;
        robot2D::VertexBuffer::Ptr m_vertexBuffer;
        BatchVertex* batchHead{nullptr};
        mutable BatchVertex* batchElement;
    };

    template<typename BatchVertex>
    QuadBatchRender<BatchVertex>::~QuadBatchRender() {
        delete[] batchHead;
    }

    template<typename BatchVertex>
    void QuadBatchRender<BatchVertex>::initalize(robot2D::AttributeLayout layout) {
        batchHead = new BatchVertex[maxVertexCount];
        assert(batchHead != nullptr && "Batch buffer must not nullptr");

        m_vertexArray = robot2D::VertexArray::Create();
        m_vertexBuffer = robot2D::VertexBuffer::Create(sizeof(BatchVertex) * maxVertexCount);
        m_vertexBuffer -> setAttributeLayout(layout);
        m_vertexArray -> setVertexBuffer(m_vertexBuffer);

        auto quadIndices = new uint32_t[maxIndexCount];
        assert(quadIndices != nullptr && "Indices buffer must not nullptr");

        uint32_t offset = 0;
        for (uint32_t i = 0; i < maxIndexCount; i += 6) {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }
        auto indexBuffer = robot2D::IndexBuffer::Create(quadIndices,
                                                        maxIndexCount);
        m_vertexArray->setIndexBuffer(indexBuffer);

        delete[] quadIndices;
    }

    template<typename BatchVertex>
    void QuadBatchRender<BatchVertex>::pack(BatchVertex&& vertex) {
        batchElement[batchElementIndex] = std::move(vertex);
        batchElementIndex++;
        cutQuadCntr++;
        if(cutQuadCntr == quadSize) {
            cutQuadCntr = 0;
            indexCount += quadIndicesSize;
        }
    }

    template<typename BatchVertex>
    void QuadBatchRender<BatchVertex>::preProcessBatching() {
        batchElement = batchHead;
    }

    template<typename BatchVertex>
    void QuadBatchRender<BatchVertex>::processBatching() {
        auto size = uint32_t((uint8_t *) batchElement - (uint8_t *)batchHead);
        m_vertexBuffer -> setData(batchHead, sizeof(BatchVertex) * batchElementIndex );
    }

    template<typename BatchVertex>
    void QuadBatchRender<BatchVertex>::refresh() {
        indexCount = 0;
        batchElementIndex = 0;
    }

    template<typename BatchVertex>
    int QuadBatchRender<BatchVertex>::getIndexCount() const {
        return indexCount;
    }

    template<typename BatchVertex>
    bool QuadBatchRender<BatchVertex>::needRefresh() const {
        return indexCount >= maxIndexCount;
    }

    template<typename BatchVertex>
    void QuadBatchRender<BatchVertex>::Bind() {
        m_vertexArray -> Bind();
    }

    template<typename BatchVertex>
    void QuadBatchRender<BatchVertex>::unBind() {
        m_vertexArray -> unBind();
    }

}
