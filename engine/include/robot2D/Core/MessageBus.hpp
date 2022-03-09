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
#include <queue>
#include <cassert>

#include <robot2D/Util/Logger.hpp>
#include "memory/MemoryManager.hpp"
#include "Message.hpp"

namespace robot2D {

    class ROBOT2D_EXPORT_API MessageBus final {
    public:
        MessageBus();
        ~MessageBus() = default;

        bool pollMessages(Message& message);

        template<typename T>
        T* postMessage(Message::ID);

        void clear();
    private:
        std::vector<void*> clearBuffer;
        std::queue<Message> m_messageQueue;
        // max value if message queue //
        unsigned int messageMax = 10000;
        mem::MemoryManager memoryManager;
    };

    template<typename T>
    T* MessageBus::postMessage(Message::ID messageType) {
        size_t messageSize = sizeof(T);
        assert(messageSize <= 128);
        Message msg{};

        msg.buffer = memoryManager.Allocate(messageSize);
        if(msg.buffer == nullptr) {
            std::cerr << "can' allocate memory for message buffer" << std::endl;
            return nullptr;
        }
        msg.id = messageType;
        msg.buffer_sz = messageSize;
        m_messageQueue.push(msg);
        clearBuffer.emplace_back(msg.buffer);
        return static_cast<T*>(msg.buffer);
    }

}