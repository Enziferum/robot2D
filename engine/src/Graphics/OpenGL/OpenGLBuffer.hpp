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

#include <robot2D/Graphics/Buffer.hpp>

namespace robot2D {
    class ROBOT2D_EXPORT_API OpenGLVertexBuffer final:  public VertexBuffer {
    public:
        OpenGLVertexBuffer(const uint32_t& size);
        OpenGLVertexBuffer(float* data, const uint32_t& size);
        virtual ~OpenGLVertexBuffer() override;

        void Bind() override;
        void unBind() override;

        void setData(const void* data, const uint32_t &size) override;
        void setAttributeLayout(const AttributeLayout& layout) override { m_layout = layout;}
        const AttributeLayout& getAttributeLayout() const override  { return m_layout;}
    private:
        unsigned m_bufferID;
        AttributeLayout m_layout;
    };

    class ROBOT2D_EXPORT_API OpenGLIndexBuffer final: public IndexBuffer {
    public:
        OpenGLIndexBuffer(uint32_t* data, const uint32_t& size);
        ~OpenGLIndexBuffer();

        void Bind() override;
        void unBind() override;
    private:
        unsigned m_bufferID;
    };
}
