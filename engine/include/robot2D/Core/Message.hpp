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

#include <robot2D/Config.hpp>

namespace robot2D {
    using MessageBuffer = void*;

    /**
     * \brief Allow to store Custom Data in 'Message' format and use it with MessageBus.
     */
    class ROBOT2D_EXPORT_API Message final {
    public:

        using ID = int32_t;
        /// Predefined Internal Message format using into Engine.
        enum MessageType {
            System = 0,
            Count
        };

        /// Message ID. Allow to check what type it is. For your format start from MessageType::Count and higher value
        ID id = -1;

        /// Allow to cast Message as your own Format returning from MessageBus
        template<typename T>
        const T& getData() const;
    private:
        template<typename Allocator, std::size_t size>
        friend class TMessageBus;
        /// Data of Message
        MessageBuffer m_buffer;
        std::size_t m_buffersz;
    };

    template<typename T>
    const T& Message::getData() const {
        //assert(sizeof(T) == buffer_sz && "T size must be == buffer_sz");
        return *static_cast<T*>(m_buffer);
    }
}