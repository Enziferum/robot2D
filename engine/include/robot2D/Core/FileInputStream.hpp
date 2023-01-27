#pragma once
#include <memory>
#include <fstream>

namespace robot2D {
    struct FileInputStream {
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