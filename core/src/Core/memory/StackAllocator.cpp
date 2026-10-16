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

    StackAllocator::StackAllocator(size_t memSize, const void* mem) :
        IAllocator(memSize, mem) {
    }

    StackAllocator::~StackAllocator() noexcept{
        clear();
    }

    void* StackAllocator::allocate(size_t memSize, u8 alignment) {
        if(memSize == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0)
            return nullptr;
        const auto current = reinterpret_cast<uptr>(m_MemoryFirstAddress) + m_MemoryUsed;
        const auto adjustment = GetAdjustment(reinterpret_cast<void*>(current), alignment, sizeof(AllocMetaInfo));
        const auto available = m_MemorySize - m_MemoryUsed;
        if(adjustment > available || memSize > available - adjustment)
            return nullptr;

        const auto aligned = current + adjustment;
        auto* metadata = reinterpret_cast<AllocMetaInfo*>(aligned - sizeof(AllocMetaInfo));
        metadata -> adjustment = adjustment;
        m_MemoryUsed += memSize + adjustment;
        ++m_MemoryAllocations;
        return reinterpret_cast<void*>(aligned);
    }

    void StackAllocator::deallocate(void* mem) {
        assert(mem != nullptr && m_MemoryAllocations > 0);
        const auto aligned = reinterpret_cast<uptr>(mem);
        const auto* metadata = reinterpret_cast<const AllocMetaInfo*>(aligned - sizeof(AllocMetaInfo));
        m_MemoryUsed = aligned - metadata -> adjustment - reinterpret_cast<uptr>(m_MemoryFirstAddress);
        --m_MemoryAllocations;
    }

    void StackAllocator::clear() noexcept{
        // simply reset memory
        m_MemoryUsed = 0;
        m_MemoryAllocations = 0;
    }
} // namespace robot2D::mem