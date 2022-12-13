#include <robot2D/Core/FileInputStream.hpp>

namespace robot2D {

    void FileInputStream::FileCloser::operator()(std::FILE* file) {
        std::fclose(file);
    }

    bool FileInputStream::openFromFile(std::string path) {
        m_file.reset(std::fopen(path.c_str(), "rb"));
        return m_file != nullptr;
    }

    std::uint64_t FileInputStream::read(void* buffer, std::uint64_t size) {
        if (m_file)
            return static_cast<std::int64_t>(
                    std::fread(buffer, 1,
                               static_cast<std::size_t>(size), m_file.get()));
        else
            return -1;
    }

    std::uint64_t FileInputStream::seek(std::uint64_t position) {
        if (m_file)
        {
            if (std::fseek(m_file.get(), static_cast<long>(position), SEEK_SET))
                return -1;

            return tell();
        }
        else
            return -1;
    }

    std::uint64_t FileInputStream::tell() {
        if (m_file)
            return std::ftell(m_file.get());
        else
            return -1;
    }

    std::uint64_t FileInputStream::getSize() {
        if (m_file)
        {
            std::int64_t position = tell();
            std::fseek(m_file.get(), 0, SEEK_END);
            std::int64_t size = tell();

            if (seek(position) == -1)
                return -1;

            return size;
        }
        else
            return -1;
    }
}