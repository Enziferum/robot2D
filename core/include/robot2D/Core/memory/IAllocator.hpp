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

#pragma once


#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <robot2D/Config.hpp>

namespace robot2D::mem {
    using u8 = uint8_t;
    using u64 = uint64_t;
    using uptr = uintptr_t ;

    class ROBOT2D_EXPORT_API IAllocator {
    public:
        IAllocator(std::size_t memSize, const void* mem);
        virtual ~IAllocator() noexcept;

        virtual void* allocate(size_t size, u8 alignment) = 0;
        virtual void deallocate(void* p) = 0;
        virtual void clear() = 0;

        inline size_t GetMemorySize() const
        {
            return m_MemorySize;
        }

        inline const void* GetMemoryAddress0() const
        {
            return m_MemoryFirstAddress;
        }

        inline size_t GetUsedMemory() const
        {
            return m_MemoryUsed;
        }

        inline u64 GetAllocationCount() const
        {
            return m_MemoryAllocations;
        }
    protected:
        const size_t m_MemorySize;
        const void*	m_MemoryFirstAddress;

        size_t	m_MemoryUsed;
        u64	m_MemoryAllocations;
    };

} // namespace robot2D::mem
