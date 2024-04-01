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