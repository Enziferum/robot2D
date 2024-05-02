/*********************************************************************
(c) Alex Raag 2024
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

#include <cstring>
#include <editor/Buffer.hpp>

namespace editor {
    template<>
    void Buffer::copy2Buffer(const int& value) {
        std::memcpy(m_buffer + m_offset, &value, sizeof(int));
        m_offset += sizeof(int);
    }

    template<>
    void Buffer::copy2Buffer(const std::string& value) {
        std::memcpy(m_buffer + m_offset, value.data(), value.length());
        m_buffer[m_offset + value.length()] = '\0';
        m_offset += value.length() + 1;
    }

    template<>
    void Buffer::unpackFromBuffer(StringBuffer& value) {
        std::memcpy(&value.str_sz, m_buffer + m_offset, sizeof(int));
        m_offset += sizeof(int);
        value.str = m_buffer + m_offset;
        m_offset += value.str_sz + 1;
    }

    void pack_message_string(const std::string& str, Buffer& buffer) {
        int str_sz = str.length();
        buffer.copy2Buffer(str_sz);
        buffer.copy2Buffer(str);
    }
}