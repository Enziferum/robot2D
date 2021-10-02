//
// Created by Necromant on 10.09.2021.
//

#include <robot2D/Util/Logger.hpp>
#include "MemoryManager.hpp"

namespace mem {

    MemoryManager::MemoryManager()
    {
        std::cout << "Initialize MemoryManager!" << std::endl;

        // allocate global memory
        this->m_GlobalMemory = malloc(MemoryManager::MEMORY_CAPACITY);
        if (this->m_GlobalMemory != nullptr)
        {
            std::cout << "%d bytes of memory allocated. " <<  MemoryManager::MEMORY_CAPACITY << std::endl;
        }
        else
        {
            std::cerr << "Failed to allocate %d bytes of memory! " << MemoryManager::MEMORY_CAPACITY << std::endl;
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
        std::cout << "Releasing MemoryManager! \n";

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
            std::cerr << "!!!  M E M O R Y   L E A K   D E T E C T E D  !!!" << std::endl;
            std::cerr << "!!!  M E M O R Y   L E A K   D E T E C T E D  !!!" << std::endl;
            std::cerr << "!!!  M E M O R Y   L E A K   D E T E C T E D  !!!" << std::endl;


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
                    std::cerr << "\'%s\' memory user didn't release allocated memory %p!" <<  i.first << i.second << std::endl;
                }
            }
        }
    }
}