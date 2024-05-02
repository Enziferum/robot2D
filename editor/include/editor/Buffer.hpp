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

#pragma once
#include <string>

namespace editor {

#pragma pack(push, 1)
    struct StringBuffer {
        int str_sz{ 0 };
        char* str{ nullptr };

        static int calcAllocSize(const std::string& str) { return str.length() + 1 + sizeof(int); }
        static int calcAllocSize(std::initializer_list<std::string> elems) {
            int allocSize = 0;
            for (const auto& str : elems)
                allocSize += str.length() + 1 + sizeof(int);
            return allocSize;
        }

        std::string as_str() {
            return std::string(str, str_sz);
        }
    };
#pragma pack(pop)



    struct Buffer {
        using byte = char;
        Buffer() = default;
        Buffer(void* rawBuffer) : m_buffer{ static_cast<byte*>(rawBuffer) } {}
        ~Buffer() = default;

        void setRawBuffer(void* rawBuffer) {
            m_offset = 0;
            m_buffer = static_cast<byte*>(rawBuffer);
        }

        template<typename T>
        void copy2Buffer(const T& value);

        template<typename T>
        void unpackFromBuffer(T&);
    private:
        byte* m_buffer{ nullptr };
        int  m_offset{ 0 };
    };

    void pack_message_string(const std::string& str, Buffer& buffer);
}