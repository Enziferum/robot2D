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
            auto position = static_cast<std::int64_t>(tell());
            std::fseek(m_file.get(), 0, SEEK_END);
            auto size = static_cast<std::int64_t>(tell());

            if (seek(position) == -1)
                return -1;

            return size;
        }
        else
            return -1;
    }
}