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

#include <robot2D/Core/memory/Allocators.hpp>

namespace mem {

    IAllocator::IAllocator(const size_t memSize, const void* mem) :
            m_MemorySize(memSize),
            m_MemoryFirstAddress(mem),
            m_MemoryUsed(0),
            m_MemoryAllocations(0)
    {}

    IAllocator::~IAllocator()
    {}


    StackAllocator::StackAllocator(size_t memSize, const void* mem) :
            IAllocator(memSize, mem)
    {}

    StackAllocator::~StackAllocator()
    {
        this->clear();
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

        asVoidPtr = (void*)this->m_MemoryFirstAddress;

        // current address
        asUptr += this->m_MemoryUsed;

        u8 adjustment = GetAdjustment(asVoidPtr, alignment, sizeof(AllocMetaInfo));

        // check if there is enough memory available
        if (this->m_MemoryUsed + memSize + adjustment > this->m_MemorySize)
        {
            // not enough memory
            return nullptr;
        }

        // store alignment in allocation meta info
        asMeta->adjustment = adjustment;

        // determine aligned memory address
        asUptr += adjustment;

        // update book keeping
        this->m_MemoryUsed += memSize + adjustment;
        this->m_MemoryAllocations++;

        // return aligned memory address
        return asVoidPtr;
    }

    void StackAllocator::free(void* mem)
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
        this->m_MemoryUsed -= ((uptr)this->m_MemoryFirstAddress + this->m_MemoryUsed) - ((uptr)mem + asMeta->adjustment);

        // decrement allocation count
        this->m_MemoryAllocations--;
    }

    void StackAllocator::clear()
    {
        // simply reset memory
        this->m_MemoryUsed = 0;
        this->m_MemoryAllocations = 0;
    }

}