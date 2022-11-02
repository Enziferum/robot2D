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

    namespace Utils {

        static GLenum TextureTarget(bool multiSampled) {
            return multiSampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        }

        static void createTextures(bool multiSampled, RenderID* outID, uint32_t count) {
            // TODO: @a.raag support OpenGL 3.3 stuff and MACOS
            glCreateTextures(TextureTarget(multiSampled), count, outID);
        }

        static void bindTexture(bool multiSampled, RenderID id) {
            glBindTexture(TextureTarget(multiSampled), id);
        }

        static void AttachColorTexture(RenderID id, int samples, GLenum internalFormat,
                                       GLenum format, robot2D::vec2i size, int index) {
            bool multiSampled = samples > 1;
            if(multiSampled) {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, size.x, size.y, GL_FALSE);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.x, size.y, 0, format, GL_UNSIGNED_BYTE, nullptr);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multiSampled), id, 0);
        }

        static void AttachDepthTexture(RenderID id, int samples, GLenum format, GLenum attachmentType,
                                       robot2D::vec2i size) {
            bool multiSampled = samples > 1;
            if(multiSampled) {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, size.x, size.y, GL_FALSE);
            } else {
                glTexStorage2D(GL_TEXTURE_2D, 1, format, size.x, size.y);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multiSampled), id, 0);
        }

        static bool isDepthFormat(FrameBufferTextureFormat format) {
            switch(format) {
                case FrameBufferTextureFormat::DEPTH24STENCIL8:
                    return true;
                default:
                    return false;
            }
        }
    }

    OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& specification):
        m_specification{specification},
        m_renderID(0)
        {

        for(auto spec: m_specification.attachments.m_attachments) {
            if(!Utils::isDepthFormat(spec.m_format)) {
                m_colorAttachementSpecs.emplace_back(spec);
            } else
                m_depthAttachmentSpec = spec;
        }

        Invalidate();
    }

    OpenGLFrameBuffer::~OpenGLFrameBuffer() {
        deleteGLStuff();
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
            deleteGLStuff();
        }

// TODO: @a.raag support OpenGL 3.3 stuff and MACOS

#if defined(ROBOT2D_WINDOWS) || defined(ROBOT2D_LINUX)
        glCreateFramebuffers(1, &m_renderID);
#endif

        glBindFramebuffer(GL_FRAMEBUFFER, m_renderID);

        // Attachments
        bool multiSampled = m_specification.Samples > 1;

        if(!m_colorAttachementSpecs.empty()) {
            m_colorAttachments.resize(m_colorAttachementSpecs.size());
            Utils::createTextures(multiSampled, m_colorAttachments.data(), m_colorAttachments.size());

            for(std::size_t it = 0; it < m_colorAttachments.size(); ++it) {
                Utils::bindTexture(multiSampled, m_colorAttachments[it]);
                switch(m_colorAttachementSpecs[it].m_format) {
                    case FrameBufferTextureFormat::RGBA8:
                        Utils::AttachColorTexture(m_colorAttachments[it], m_specification.Samples,
                                                  GL_RGBA8, GL_RGBA,
                                                  m_specification.size, it);
                        break;
                    case FrameBufferTextureFormat::RED_INTEGER:
                        Utils::AttachColorTexture(m_colorAttachments[it], m_specification.Samples,
                                                  GL_R32I, GL_RED_INTEGER,
                                                  m_specification.size, it);
                        break;
                }
            }
        }

        if(m_depthAttachmentSpec.m_format != FrameBufferTextureFormat::None) {
            Utils::createTextures(multiSampled, &m_depthAttachment, 1);
            Utils::bindTexture(multiSampled, m_depthAttachment);
            switch(m_depthAttachmentSpec.m_format) {
                case FrameBufferTextureFormat::DEPTH24STENCIL8:
                    Utils::AttachDepthTexture(m_depthAttachment, m_specification.Samples,
                                              GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT,
                                              m_specification.size);
                    break;
            }
        }

        if(m_colorAttachments.size() > 1) {
            // TODO assert <= 4
            GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                  GL_COLOR_ATTACHMENT2,
                                  GL_COLOR_ATTACHMENT3};
            glDrawBuffers(m_colorAttachments.size(), buffers);
        } else if(m_colorAttachments.empty()) {
            // only depth pass
            glDrawBuffer(GL_NONE);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    const FrameBufferSpecification &OpenGLFrameBuffer::getSpecification() const {
        return m_specification;
    }

    FrameBufferSpecification &OpenGLFrameBuffer::getSpecification() {
        return m_specification;;
    }

    const RenderID& OpenGLFrameBuffer::getFrameBufferRenderID(uint32_t index) const {
        // TODO: assert size
        return m_colorAttachments[index];
    }

    void OpenGLFrameBuffer::Resize(const vec2u& newSize) {
        m_specification.size = {static_cast<int>(newSize.x),
                                static_cast<int>(newSize.y)};
        Invalidate();
    }

    void OpenGLFrameBuffer::deleteGLStuff() {
        glDeleteFramebuffers(1, &m_renderID);
        glDeleteTextures(m_colorAttachments.size(), m_colorAttachments.data());
        glDeleteTextures(1, &m_depthAttachment);

        m_colorAttachments.clear();
        m_depthAttachment = 0;
    }

    int OpenGLFrameBuffer::readPixel(RenderID attachmentIndex, vec2i mousePos) {
        // TODO: @a.raag assert for ID
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
        int pixelData;
        glReadPixels(mousePos.x, mousePos.y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
        return pixelData;
    }
}