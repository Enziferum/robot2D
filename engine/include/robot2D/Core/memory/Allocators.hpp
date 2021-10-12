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
#include <cstdint>
#include <cassert>
namespace mem {
    using u8 = uint8_t;
    using u64 = uint64_t;
    using uptr = uintptr_t ;


    static inline void* AlignForward(void* address, u8 alignment)
    {
        return (void*)((reinterpret_cast<uptr>(address)+static_cast<uptr>(alignment - 1)) & static_cast<uptr>(~(alignment - 1)));
    }

    // returns the number of bytes needed to align the address
    static inline u8 GetAdjustment(const void* address, u8 alignment)
    {
        u8 adjustment = alignment - (reinterpret_cast<uptr>(address)& static_cast<uptr>(alignment - 1));

        return adjustment == alignment ? 0 : adjustment;
    }

    static inline u8 GetAdjustment(const void* address, u8 alignment, u8 extra)
    {
        u8 adjustment = GetAdjustment(address, alignment);

        u8 neededSpace = extra;

        if (adjustment < neededSpace)
        {
            neededSpace -= adjustment;

            //Increase adjustment to fit header
            adjustment += alignment * (neededSpace / alignment);

            if (neededSpace % alignment > 0)
                adjustment += alignment;
        }

        return adjustment;
    }


    class IAllocator {
    public:
        IAllocator(const size_t memSize, const void* mem);
        virtual ~IAllocator();

        virtual void* allocate(size_t size, u8 alignment) = 0;
        virtual void free(void* p) = 0;
        virtual void clear() = 0;

        inline size_t GetMemorySize() const
        {
            return this->m_MemorySize;
        }

        inline const void* GetMemoryAddress0() const
        {
            return this->m_MemoryFirstAddress;
        }

        inline size_t GetUsedMemory() const
        {
            return this->m_MemoryUsed;
        }

        inline u64 GetAllocationCount() const
        {
            return this->m_MemoryAllocations;
        }
    protected:

        const size_t		m_MemorySize;
        const void*			m_MemoryFirstAddress;

        size_t				m_MemoryUsed;
        u64					m_MemoryAllocations;
    };


    class StackAllocator : public IAllocator
    {
    public:

        StackAllocator(size_t memSize, const void* mem);

        virtual ~StackAllocator();

        virtual void* allocate(size_t size, u8 alignment) override;
        virtual void free(void* p) override;
        virtual void clear() override;

    private:
        struct AllocMetaInfo
        {
            u8 adjustment;
        };
    };

}