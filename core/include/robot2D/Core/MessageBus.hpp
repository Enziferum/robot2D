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
#include <queue>
#include <cassert>

#include <robot2D/Util/Logger.hpp>
#include "memory/StackAllocator.hpp"
#include "memory/MemoryManager.hpp"
#include "Message.hpp"

namespace robot2D {
    constexpr const unsigned int messageMax = 10000;

    struct BadMessageAllocation: public std::exception {
        const char* what() const noexcept override {
            return "robot2D::Message bad allocation";
        }
    };

    /**
     * \brief Postman for custom Data as Message.
     * \details To communicate with different parts of your project you can use  \n
     * either 'callbacks' or notifications / messages.
     * Post Message is for flexible process if compare to callbacks because it's allow to don't worry \name
     * about callbacks memory / lifetime and catched objects.
    */
    template<typename Allocator = mem::StackAllocator, std::size_t maxMessageSize = 128>
    class ROBOT2D_EXPORT_API TMessageBus {
    public:
        ///\brief Only initialize MessageBus.
        ///\details to pack / post message use postMessage method.
        TMessageBus();
        TMessageBus(const TMessageBus& other) = delete;
        TMessageBus& operator=(const TMessageBus& other) = delete;
        TMessageBus(TMessageBus&& other) = delete;
        TMessageBus& operator=(TMessageBus&& other) = delete;
        ~TMessageBus() = default;

        /// Process input message and deliver to your Message Handler
        bool pollMessages(Message& message);

        /// Push your message into processing queue
        template<typename T>
        T* postMessage(Message::ID);

        void* postMessage(Message::ID, std::size_t allocSize);

        /// Clear internal buffers
        void clear();

        bool empty() const noexcept;

        std::size_t pendingMessages() const noexcept;
    private:
        mem::MemoryManager<Allocator> m_memoryManager;
        std::vector<void*> m_clearBuffer;
        std::queue<Message> m_messageQueue;
    };

    /// \brief MessageBus with default allocator and default message size
    using MessageBus = TMessageBus<>;

    template<typename Allocator, std::size_t size>
    TMessageBus<Allocator, size>::TMessageBus():
            m_memoryManager{},
            m_messageQueue()
    {
        m_clearBuffer.reserve(messageMax);
    }

    template<typename Allocator, std::size_t size>
    bool TMessageBus<Allocator, size>::pollMessages(Message& message) {
        if(m_messageQueue.empty()) {
            return false;
        }

        message = m_messageQueue.front();
        m_messageQueue.pop();

        return true;
    }

    template<typename Allocator, std::size_t size>
    void TMessageBus<Allocator, size>::clear() {
        for(auto& it: m_clearBuffer) {
            if(it != nullptr)
                m_memoryManager.deallocate(it);
        }

        m_clearBuffer.clear();
        /// ?????
        m_memoryManager.checkLeaks();
    }


    template<typename Allocator, std::size_t size>
    template<typename T>
    T* TMessageBus<Allocator, size>::postMessage(Message::ID messageType) {
        std::size_t messageSize = sizeof(T);
        assert(messageSize <= size && "Message ");
        Message msg{};

        try {
            msg.m_buffer = m_memoryManager.allocate(messageSize);
            if(msg.m_buffer == nullptr) {
                std::cerr << "can' allocate memory for message buffer" << std::endl;
                return nullptr;
            }
        }
        catch(...) {
            /// bad alloc or smth ///
        }

        msg.id = messageType;
        msg.m_buffersz = messageSize;
        m_messageQueue.push(msg);
        m_clearBuffer.emplace_back(msg.m_buffer);
        return static_cast<T*>(msg.m_buffer);
    }


    template<typename Allocator, std::size_t size>
    void* TMessageBus<Allocator, size>::postMessage(Message::ID messageType, std::size_t allocSize) {
        Message msg{};
        msg.m_needUnpack = true;

        try {
            msg.m_buffer = m_memoryManager.allocate(allocSize);
            if (msg.m_buffer == nullptr) {
                std::cerr << "can' allocate memory for message buffer" << std::endl;
                return nullptr;
            }
        }
        catch (...) {
            /// bad alloc or smth ///
        }

        msg.id = messageType;
        msg.m_buffersz = allocSize;
        m_messageQueue.push(msg);
        m_clearBuffer.emplace_back(msg.m_buffer);
        return msg.m_buffer;
    }


    template<typename Allocator, std::size_t maxMessageSize>
    bool TMessageBus<Allocator, maxMessageSize>::empty() const noexcept {
        return m_messageQueue.empty();
    }

    template<typename Allocator, std::size_t maxMessageSize>
    std::size_t TMessageBus<Allocator, maxMessageSize>::pendingMessages() const noexcept {
        return m_messageQueue.size();
    }

}