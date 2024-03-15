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

#include <robot2D/Graphics/FrameBuffer.hpp>

namespace robot2D::priv {
    class OpenGLFrameBuffer final: public FrameBuffer {
    public:
        OpenGLFrameBuffer(const FrameBufferSpecification& specification);
        ~OpenGLFrameBuffer() override;

        void Bind() override;
        void unBind() override;
        void Invalidate() override;

        int readPixel(RenderID attachmentIndex, vec2i mousePos) override;

        void Resize(const robot2D::vec2u& newSize) override;
        const RenderID& getFrameBufferRenderID(uint32_t index = 0) const override;
        const FrameBufferSpecification& getSpecification() const override;

        FrameBufferSpecification& getSpecification() override;
    private:
        void deleteGLStuff();
    private:
        FrameBufferSpecification m_specification;
        RenderID m_renderID;

        std::vector<FrameBufferTextureSpecification> m_colorAttachementSpecs;
        FrameBufferTextureSpecification m_depthAttachmentSpec = FrameBufferTextureFormat::None;

        std::vector<RenderID> m_colorAttachments;
        RenderID m_depthAttachment;
    };
}