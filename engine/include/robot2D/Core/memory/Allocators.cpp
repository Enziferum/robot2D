//
// Created by Necromant on 10.09.2021.
//

#include "Allocators.hpp"

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