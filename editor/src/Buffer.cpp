#include <editor/Buffer.hpp>

namespace editor {
    template<>
    void Buffer::copy2Buffer(const int& value) {
        memcpy(m_buffer + m_offset, &value, sizeof(int));
        m_offset += sizeof(int);
    }

    template<>
    void Buffer::copy2Buffer(const std::string& value) {
        memcpy(m_buffer + m_offset, value.data(), value.length());
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