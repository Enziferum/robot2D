/*********************************************************************
(c) Alex Raag 2023
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

#include <robot2D/Core/memory/StackAllocator.hpp>

namespace robot2D::mem {

    StackAllocator::StackAllocator(size_t memSize, const void* mem):
            IAllocator(memSize, mem)
    {}

    StackAllocator::~StackAllocator() noexcept
    {
        clear();
    }

    void* StackAllocator::allocate(size_t memSize, u8 alignment)
    {
        assert(memSize > 0 && "allocate called with memSize = 0.");

        union
        {
            void* asVoidPtr;
            uptr asUptr;
            AllocMetaInfo* asMeta;
        };

        asVoidPtr = (void*)m_MemoryFirstAddress;

        // current address
        asUptr += m_MemoryUsed;

        u8 adjustment = GetAdjustment(asVoidPtr, alignment, sizeof(AllocMetaInfo));

        // check if there is enough memory available
        if (m_MemoryUsed + memSize + adjustment > m_MemorySize)
        {
            // not enough memory
            return nullptr;
        }

        // store alignment in allocation meta info
        asMeta -> adjustment = adjustment;

        // determine aligned memory address
        asUptr += adjustment;

        // update book keeping
        m_MemoryUsed += memSize + adjustment;
        m_MemoryAllocations++;

        // return aligned memory address
        return asVoidPtr;
    }

    void StackAllocator::deallocate(void* mem)
    {
        union
        {
            void* asVoidPtr;
            uptr asUptr;
            AllocMetaInfo* asMeta;
        };

        asVoidPtr = mem;

        // get meta info
        asUptr -= sizeof(AllocMetaInfo);

        // free used memory
        m_MemoryUsed -= ((uptr)m_MemoryFirstAddress + m_MemoryUsed) - ((uptr)mem + asMeta->adjustment);

        // decrement allocation count
        m_MemoryAllocations--;
    }

    void StackAllocator::clear() noexcept
    {
        // simply reset memory
        m_MemoryUsed = 0;
        m_MemoryAllocations = 0;
    }
} // namespace robot2D::mem