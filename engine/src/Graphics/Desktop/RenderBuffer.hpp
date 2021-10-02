//
// Created by Necromant on 29.09.2021.
//

#pragma once
#include <array>
#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Graphics/Buffer.hpp>
#include <robot2D/Graphics/VertexArray.hpp>

namespace robot2D {

#pragma pack(push, 1)
    struct RenderVertex {
        robot2D::vec2f Position;
        robot2D::vec2f TextureCoords;
        Color Color;
        float textureIndex;
    };
#pragma pack(pop)


    struct RenderBuffer {
        unsigned int maxQuadsCount = 20000;
        unsigned int maxVerticesCount = maxQuadsCount * 4;
        unsigned int maxIndicesCount = maxQuadsCount * 6;

        VertexArray::Ptr vertexArray;
        VertexBuffer::Ptr vertexBuffer;

        RenderVertex* quadBuffer = nullptr;
        RenderVertex* quadBufferPtr = nullptr;

        Texture whiteTexture;
        std::array<uint32_t, 32> textureSlots;

        uint32_t indexCount = 0;
        uint32_t textureSlotIndex = 1;

        robot2D::vec2f quadVertexPositions[4];

        unsigned int VAO;
        unsigned int VBO;
        unsigned int EBO;
    };
}