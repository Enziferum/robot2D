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

#include <memory>
#include <fstream>

namespace robot2D {

    class FileInputStream {
    public:
        FileInputStream() = default;
        FileInputStream(const FileInputStream& other) = delete;
        FileInputStream& operator=(const FileInputStream& other) = delete;
        FileInputStream(FileInputStream&& other) = delete;
        FileInputStream& operator=(FileInputStream&& other) = delete;
        ~FileInputStream() = default;

        /// \brief open's file as binary in reading mode only
        bool openFromFile(std::string path);

        /// \brief Read's buffer from File and move cursor byself.
        std::uint64_t read(void* buffer, std::uint64_t size);

        /// \brief Move file's actual cursor. If result == -1 endof file or can't get.
        std::uint64_t seek(std::uint64_t position);

        /// \brief Get file's actual cursor. If result == -1 endof file or can't get.
        std::uint64_t tell();

        /// \brief Get file size.
        std::uint64_t getSize();
    private:
        struct FileCloser {
            void operator()(std::FILE*);
        };

        std::unique_ptr<std::FILE, FileCloser> m_file{nullptr};
    };
}