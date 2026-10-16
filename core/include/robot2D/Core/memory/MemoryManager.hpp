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

#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <cstddef>
#include <new>

#include <robot2D/Config.hpp>
#include "IAllocator.hpp"

namespace robot2D::mem {

    using memoryBuffer = void*;

    template<typename Allocator>
    class ROBOT2D_EXPORT_API MemoryManager {
    public:
        static constexpr std::size_t MEMORY_CAPACITY = 1024 * 1024; // 1 MiB arena
    public:
        MemoryManager();
        MemoryManager(const MemoryManager&) = delete;
        MemoryManager& operator=(const MemoryManager&) = delete;
        MemoryManager(MemoryManager&&) = delete;
        MemoryManager& operator=(MemoryManager&&) = delete;
        ~MemoryManager() noexcept;

        inline void* allocate(std::size_t memSize, const char* user = nullptr,
            u8 alignment = alignof(std::max_align_t)) {
            void* pMemory = m_MemoryAllocator -> allocate(memSize, alignment);
            if(pMemory) {
                try {
                    m_PendingMemory.emplace_back(user, pMemory);
                }
                catch(...) {
                    m_MemoryAllocator -> deallocate(pMemory);
                    throw;
                }
            }
            return pMemory;
        }

        /// Releases allocations in stack order, deferring earlier releases until possible.
        inline void deallocate(void* pMem) {
            if(!pMem || m_PendingMemory.empty())
                return;
            auto pending = std::find_if(m_PendingMemory.begin(), m_PendingMemory.end(),
                    [pMem](const auto& entry) { return entry.second == pMem; });
            if(pending == m_PendingMemory.end())
                return;
            if(pMem != m_PendingMemory.back().second) {
                if(std::find(m_FreedMemory.begin(), m_FreedMemory.end(), pMem) == m_FreedMemory.end())
                    m_FreedMemory.push_back(pMem);
                return;
            }
            m_MemoryAllocator -> deallocate(pMem);
            m_PendingMemory.pop_back();
            m_FreedMemory.remove(pMem);
            while(!m_PendingMemory.empty()) {
                auto* last = m_PendingMemory.back().second;
                auto released = std::find(m_FreedMemory.begin(), m_FreedMemory.end(), last);
                if(released == m_FreedMemory.end())
                    break;
                m_MemoryAllocator -> deallocate(last);
                m_PendingMemory.pop_back();
                m_FreedMemory.erase(released);
            }
        }

        void checkMemoryLeaks();
    private:
        /// \brief memory tag & memory buffer
        using memory_pair = std::pair<std::string, memoryBuffer>;

        /// \brief Pointer to global allocated memory
        void* m_GlobalMemory{ nullptr };
        /// \brief Allocator used to manager memory allocation from global memory
        Allocator* m_MemoryAllocator;
        std::vector<std::pair<const char*, memoryBuffer> > m_PendingMemory;
        std::list<memoryBuffer> m_FreedMemory;
    };


    template<typename Allocator>
    MemoryManager<Allocator>::MemoryManager() {
        m_GlobalMemory = std::malloc(MemoryManager::MEMORY_CAPACITY);
        if(!m_GlobalMemory)
            throw std::bad_alloc{};
        try {
            m_MemoryAllocator = new Allocator(MemoryManager::MEMORY_CAPACITY, m_GlobalMemory);
        }
        catch(...) {
            std::free(m_GlobalMemory);
            throw;
        }

        m_PendingMemory.clear();
        m_FreedMemory.clear();
    }

    template<typename Allocator>
    MemoryManager<Allocator>::~MemoryManager() noexcept{
        // RB_CORE_INFO("Releasing MemoryManager!");

        m_MemoryAllocator -> clear();
        delete m_MemoryAllocator;
        m_MemoryAllocator = nullptr;

        free(m_GlobalMemory);
        m_GlobalMemory = nullptr;
    }

    template<typename Allocator>
    void MemoryManager<Allocator>::checkMemoryLeaks() {
        assert(!(!m_FreedMemory.empty() && m_PendingMemory.empty()) && "Implementation failure!");

        if(!m_PendingMemory.empty()) {
            // RB_CORE_ERROR("!!!  M E M O R Y   L E A K   D E T E C T E D  !!!");
            for(auto i : m_PendingMemory) {
                bool isFreed = false;
                for(auto j : m_FreedMemory) {
                    if(i.second == j) {
                        isFreed = true;
                        break;
                    }
                }

                if(!isFreed) {
                    //  RB_CORE_ERROR("\'%s\' memory user didn't release allocated memory %p!", i.first, i.second);
                }
            }
        }
    }
} // namespace robot2D::mem