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

#include <memory>
#include <vector>
#include <robot2D/Core/Vector2.hpp>

namespace robot2D {

    using RenderID = uint32_t;

    enum class FrameBufferTextureFormat {
        None = 0,
        //Color
        RGBA8,
        RED_INTEGER,

        // Depth/Stencil
        DEPTH24STENCIL8,

        // Defaults
        Depth = DEPTH24STENCIL8
    };

    struct FrameBufferTextureSpecification {
        FrameBufferTextureSpecification() = default;
        FrameBufferTextureSpecification(FrameBufferTextureFormat format): m_format{format} {}
        ~FrameBufferTextureSpecification() = default;


        FrameBufferTextureFormat m_format;
    };

    struct FrameBufferAttachmentSpecification {
        FrameBufferAttachmentSpecification() = default;
        FrameBufferAttachmentSpecification(
                std::initializer_list<FrameBufferTextureSpecification> attachments): m_attachments(attachments) {}
        std::vector<FrameBufferTextureSpecification> m_attachments;
    };

    struct FrameBufferSpecification {
        vec2i size;
        uint32_t Samples = 1;
        FrameBufferAttachmentSpecification attachments;
        bool SwapChainTarget = false;
    };

    /**
     * Interface for Graphics API specific FrameBuffer.
     */
    class FrameBuffer {
    public:
        using Ptr = std::shared_ptr<FrameBuffer>;
    public:
        virtual ~FrameBuffer() = 0;
        virtual void Bind() = 0;
        virtual void unBind() = 0;
        virtual int readPixel(RenderID attachmentIndex, vec2i mousePos) = 0;
        virtual void Invalidate() = 0;
        virtual void Resize(const robot2D::vec2u& newSize) = 0;
        virtual const RenderID& getFrameBufferRenderID(uint32_t index = 0) const = 0;
        virtual const FrameBufferSpecification& getSpecification() const = 0;
        virtual FrameBufferSpecification& getSpecification()= 0;

        static Ptr Create(const FrameBufferSpecification& specification);
    };
}
