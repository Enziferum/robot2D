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

#include <robot2D/Graphics/GL.hpp>
#include "OpenGLFrameBuffer.hpp"

namespace robot2D::priv {

    OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& specification):
    m_specification{specification}, m_renderID(0) {
        Invalidate();
    }

    OpenGLFrameBuffer::~OpenGLFrameBuffer() {
        glDeleteFramebuffers(1, &m_renderID);
        glDeleteTextures(1, &m_colorAttachment);
        glDeleteTextures(1, &m_depthAttachment);
    }

    void OpenGLFrameBuffer::Bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_renderID);
        glViewport(0, 0, m_specification.size.x, m_specification.size.y);
    }

    void OpenGLFrameBuffer::unBind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFrameBuffer::Invalidate() {
        if(m_renderID != 0) {
            glDeleteFramebuffers(1, &m_renderID);
            glDeleteTextures(1, &m_colorAttachment);
            glDeleteTextures(1, &m_depthAttachment);
        }

        glGenFramebuffers(1, &m_renderID);
        Bind();

        glGenTextures(1, &m_colorAttachment);
        glBindTexture(GL_TEXTURE_2D, m_colorAttachment);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_specification.size.x,
                     m_specification.size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorAttachment, 0);

        glGenTextures(1, &m_depthAttachment);
        glBindTexture(GL_TEXTURE_2D, m_depthAttachment);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_specification.size.x,
                     m_specification.size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthAttachment, 0);
        unBind();
    }

    const FrameBufferSpecification &OpenGLFrameBuffer::getSpecification() const {
        return m_specification;
    }

    FrameBufferSpecification &OpenGLFrameBuffer::getSpecification() {
        return m_specification;;
    }

    const RenderID& OpenGLFrameBuffer::getFrameBufferRenderID() const {
        return m_colorAttachment;
    }

    void OpenGLFrameBuffer::Resize(const vec2u& newSize) {
        m_specification.size = {newSize.x, newSize.y};
        Invalidate();
    }


}