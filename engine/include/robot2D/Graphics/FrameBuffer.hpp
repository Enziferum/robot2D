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
#include <robot2D/Core/Vector2.hpp>

namespace robot2D {

    using RenderID = uint32_t;

    struct FrameBufferSpecification {
        vec2i size;
        uint32_t Samples = 1;
        bool SwapChainTarget = false;
    };

    class FrameBuffer {
    public:
        using Ptr = std::shared_ptr<FrameBuffer>;
    public:
        virtual ~FrameBuffer() = 0;
        virtual void Bind() = 0;
        virtual void unBind() = 0;
        virtual void Invalidate() = 0;
        virtual const RenderID& getFrameBufferRenderID() const = 0;
        virtual const FrameBufferSpecification& getSpecification() const = 0;
        virtual FrameBufferSpecification& getSpecification()= 0;

        static Ptr Create(const FrameBufferSpecification& specification);
    };
}
