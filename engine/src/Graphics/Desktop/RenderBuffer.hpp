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

#pragma once
#include <array>
#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Graphics/Buffer.hpp>
#include <robot2D/Graphics/VertexArray.hpp>

namespace robot2D {

#pragma pack(push, 1)
    struct ROBOT2D_EXPORT_API RenderVertex {
        robot2D::vec2f Position;
        Color Color;
        robot2D::vec2f TextureCoords;
        float textureIndex;
    };
#pragma pack(pop)


    struct ROBOT2D_EXPORT_API RenderBuffer {
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
        uint32_t textureSlotIndex = 0;

        robot2D::vec2f quadVertexPositions[4];
    };
}