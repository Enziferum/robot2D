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

#include <robot2D/Util/Logger.hpp>
#include <robot2D/Core/memory/MemoryManager.hpp>

namespace mem {

    MemoryManager::MemoryManager()
    {
        RB_CORE_INFO("Initialize MemoryManager!");

        // allocate global memory
        this->m_GlobalMemory = malloc(MemoryManager::MEMORY_CAPACITY);
        if (this->m_GlobalMemory != nullptr)
        {
            RB_CORE_INFO("{0} bytes of memory allocated.", MemoryManager::MEMORY_CAPACITY);
        }
        else
        {
            RB_CORE_ERROR("Failed to allocate %d bytes of memory!", MemoryManager::MEMORY_CAPACITY);
            assert(this->m_GlobalMemory != nullptr && "Failed to allocate global memory.");
        }

        // create allocator
        this->m_MemoryAllocator = new StackAllocator(MemoryManager::MEMORY_CAPACITY, this->m_GlobalMemory);
        assert(this->m_MemoryAllocator != nullptr && "Failed to create memory allocator!");

        this->m_PendingMemory.clear();
        this->m_FreedMemory.clear();
    }

    MemoryManager::~MemoryManager()
    {
        RB_CORE_INFO("Releasing MemoryManager!");

        this->m_MemoryAllocator->clear();

        delete this->m_MemoryAllocator;
        this->m_MemoryAllocator = nullptr;

        free(this->m_GlobalMemory);
        this->m_GlobalMemory = nullptr;
    }

    void MemoryManager::CheckMemoryLeaks()
    {
        assert(!(this->m_FreedMemory.size() > 0 && this->m_PendingMemory.size() == 0) && "Implementation failure!");

        if (this->m_PendingMemory.size() > 0)
        {
            RB_CORE_ERROR("!!!  M E M O R Y   L E A K   D E T E C T E D  !!!");

            for (auto i : this->m_PendingMemory)
            {
                bool isFreed = false;

                for (auto j : this->m_FreedMemory)
                {
                    if (i.second == j)
                    {
                        isFreed = true;
                        break;
                    }
                }

                if (isFreed == false)
                {
                    RB_CORE_ERROR("\'%s\' memory user didn't release allocated memory %p!", i.first, i.second);
                }
            }
        }
    }
}