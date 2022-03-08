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

#include <vector>
#include <list>
#include "Allocators.hpp"

namespace mem {

    class MemoryManager
    {
    public:
        //134217728
        static constexpr size_t MEMORY_CAPACITY = 1024 * 1024;

    public:
        MemoryManager();
        ~MemoryManager();


        inline void* Allocate(size_t memSize, const char* user = nullptr)
        {
            // LogDebug("%s allocated %d bytes of global memory.", user != nullptr ? user : "Unknown", memSize);
            void* pMemory = m_MemoryAllocator->allocate(memSize, alignof(u8));

            this->m_PendingMemory.push_back(std::pair<const char*, void*>(user, pMemory));

            return pMemory;
        }

        inline void Free(void* pMem)
        {
            if (pMem == this->m_PendingMemory.back().second)
            {
                this->m_MemoryAllocator->free(pMem);
                this->m_PendingMemory.pop_back();

                bool bCheck = true;
                while(bCheck == true)
                {
                    bCheck = false;

                    // do not report already freed memory blocks.
                    for (auto it : this->m_FreedMemory)
                    {
                        if (it == this->m_PendingMemory.back().second)
                        {
                            this->m_MemoryAllocator->free(pMem);
                            this->m_PendingMemory.pop_back();
                            this->m_FreedMemory.remove(it);

                            bCheck = true;
                            break;
                        }
                    }
                };

            }
            else
            {
                this->m_FreedMemory.push_back(pMem);
            }
        }

        void CheckMemoryLeaks();
    private:

        // Pointer to global allocated memory
        void*										m_GlobalMemory;

        // Allocator used to manager memory allocation from global memory
        StackAllocator*								m_MemoryAllocator;

        std::vector<std::pair<const char*, void*>>	m_PendingMemory;

        std::list<void*>							m_FreedMemory;

        MemoryManager(const MemoryManager&) = delete;
        MemoryManager& operator=(MemoryManager&) = delete;

    };

}