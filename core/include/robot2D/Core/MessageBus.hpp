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
#include <queue>
#include <vector>
#include <new>
#include <stdexcept>
#include <type_traits>

#include <robot2D/Util/Logger.hpp>
#include "memory/StackAllocator.hpp"
#include "memory/MemoryManager.hpp"
#include "Message.hpp"

namespace robot2D {
    constexpr const unsigned int messageMax = 10000;

    struct BadMessageAllocation : public std::bad_alloc {
        const char* what() const noexcept override {
            return "robot2D::Message bad allocation";
        }
    };

    /**
     * @brief Owns payloads and delivers messages in FIFO order.
     * @details Polling removes an envelope, but its payload remains valid until
     * clear() or destruction. Typed payloads are value-initialized and destroyed
     * by the bus. Callers must not destroy or free payloads themselves.
     * Access and mutation require external synchronization across threads.
     * @tparam Allocator Stack-compatible allocator used by MemoryManager.
     * @tparam maxMessageSize Maximum typed payload size in bytes; raw payloads may use the arena capacity.
     */
    template<typename Allocator = mem::StackAllocator, std::size_t maxMessageSize = 128>
    class ROBOT2D_EXPORT_API TMessageBus {
    public:
        TMessageBus() {
            m_clearBuffer.reserve(messageMax);
        }
        TMessageBus(const TMessageBus& other) = delete;
        TMessageBus& operator=(const TMessageBus& other) = delete;
        TMessageBus(TMessageBus&& other) = delete;
        TMessageBus& operator=(TMessageBus&& other) = delete;
        ~TMessageBus() {
            clear();
        }

        /// @brief Pops the oldest envelope; leaves message unchanged if empty.
        bool pollMessages(Message& message) {
            if(m_messageQueue.empty())
                return false;
            message = m_messageQueue.front();
            m_messageQueue.pop();
            return true;
        }

        /**
         * @brief Value-initializes an owned T and queues its envelope.
         * @return Mutable payload pointer, valid until clear() or destruction.
         * @throws std::length_error If sizeof(T) exceeds maxMessageSize.
         * @throws BadMessageAllocation If the arena is exhausted.
         * @details Construction and bookkeeping exceptions propagate without queuing a message.
         */
        template<typename T>
        T* postMessage(Message::ID id) {
            static_assert(alignof(T) <= 128, "Allocator supports alignments up to 128 bytes");
            static_assert(std::is_nothrow_destructible_v<T>, "Payload destruction must not throw");
            if(sizeof(T) > maxMessageSize)
                throw std::length_error("Typed message payload exceeds the configured size limit");
            auto* buffer = allocate(sizeof(T), static_cast<mem::u8>(alignof(T)));
            T* payload = nullptr;
            try {
                payload = new(buffer) T{};
            }
            catch(...) {
                m_memoryManager.deallocate(buffer);
                throw;
            }
            enqueue(id, payload, sizeof(T), false, [](void* ptr) { static_cast<T*>(ptr) -> ~T(); });
            return payload;
        }

        /**
         * @brief Queues uninitialized bytes for a custom MessageFormatter.
         * @return Owned storage, valid until clear() or destruction.
         * @throws std::length_error For zero size.
         * @throws BadMessageAllocation If the arena is exhausted.
         * @details The bus does not construct or destroy objects in raw storage.
         */
        void* postMessage(Message::ID id, std::size_t allocSize) {
            auto* buffer = allocate(allocSize, alignof(std::max_align_t));
            enqueue(id, buffer, allocSize, true, nullptr);
            return buffer;
        }

        /// @brief Drops all queued messages and destroys all payloads, including polled ones.
        void clear() {
            while(!m_messageQueue.empty())
                m_messageQueue.pop();
            for(auto it = m_clearBuffer.rbegin(); it != m_clearBuffer.rend(); ++it) {
                if(it -> destroy)
                    it -> destroy(it -> buffer);
                m_memoryManager.deallocate(it -> buffer);
            }
            m_clearBuffer.clear();
            m_memoryManager.checkMemoryLeaks();
        }

        /// @brief Returns whether no envelopes are pending; polled payloads may remain alive.
        bool empty() const noexcept {
            return m_messageQueue.empty();
        }
        /// @brief Returns the number of envelopes awaiting polling.
        std::size_t pendingMessages() const noexcept {
            return m_messageQueue.size();
        }
    private:
        struct PendingPayload {
            void* buffer;
            void (*destroy)(void*);
        };

        void* allocate(std::size_t bytes, mem::u8 alignment) {
            if(bytes == 0)
                throw std::length_error("Message payload size must be positive");
            auto* buffer = m_memoryManager.allocate(bytes, nullptr, alignment);
            if(!buffer)
                throw BadMessageAllocation{};
            return buffer;
        }

        void enqueue(Message::ID id, void* buffer, std::size_t bytes,
            bool unpack, void (*destroy)(void*)) {
            try {
                m_clearBuffer.push_back({buffer, destroy});
                try {
                    Message message{};
                    message.id = id;
                    message.m_buffer = buffer;
                    message.m_buffersz = bytes;
                    message.m_needUnpack = unpack;
                    m_messageQueue.push(message);
                }
                catch(...) {
                    m_clearBuffer.pop_back();
                    throw;
                }
            }
            catch(...) {
                if(destroy)
                    destroy(buffer);
                m_memoryManager.deallocate(buffer);
                throw;
            }
        }

        mem::MemoryManager<Allocator> m_memoryManager;
        std::vector<PendingPayload> m_clearBuffer;
        std::queue<Message> m_messageQueue;
    };

    /// @brief Message bus with a stack arena and a 128-byte payload limit.
    using MessageBus = TMessageBus<>;
}
