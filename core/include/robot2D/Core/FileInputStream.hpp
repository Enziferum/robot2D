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
#include <cstdio>
#include <cstdint>
#include <limits>
#include <string>

namespace robot2D {

    /** @brief Owns a binary file opened for synchronous reading.
     * Paths use UTF-8. Positions and counts are bytes; Error is distinct from EOF.
     * A stream starts closed and closes its file on destruction or any reopen.
     * Instances cannot be copied/moved and require external synchronization.
     */
    class FileInputStream {
    public:
        /// Operation failure; the existing unsigned return type is retained.
        static constexpr std::uint64_t Error = std::numeric_limits<std::uint64_t>::max();

        FileInputStream() = default;
        FileInputStream(const FileInputStream& other) = delete;
        FileInputStream& operator=(const FileInputStream& other) = delete;
        FileInputStream(FileInputStream&& other) = delete;
        FileInputStream& operator=(FileInputStream&& other) = delete;
        ~FileInputStream() = default;

        /** @brief Replaces the current file, starting at byte zero on success.
         * @return false for an invalid/unavailable path; the stream is then closed.
         * Empty paths and embedded NUL bytes are rejected. Parent directories are
         * not created. Allocation failures may propagate as exceptions.
         */
        bool openFromFile(std::string path);

        /** @brief Reads up to size bytes and advances by the bytes consumed.
         * @return Bytes read (possibly short), zero at EOF, or Error on failure.
         * An open stream accepts size == 0 with a null buffer. Otherwise buffer
         * must reference at least size writable bytes. Null buffers, counts beyond
         * size_t, closed streams and I/O failures return Error. An I/O failure may
         * leave partially written data and an advanced cursor; discard that read.
         */
        std::uint64_t read(void* buffer, std::uint64_t size);

        /** @brief Seeks to an absolute byte offset; seeking past EOF is allowed.
         * @return New position or Error (closed stream, unrepresentable offset,
         * or I/O failure). Successful seeking clears EOF and read errors.
         * Representable offsets use the platform's large-file API (64-bit on
         * supported desktop targets). The file is never extended by seeking.
         */
        std::uint64_t seek(std::uint64_t position);

        /// Returns the current byte offset, including at EOF, or Error on failure.
        std::uint64_t tell();

        /** @brief Returns the byte size while restoring the original cursor.
         * Returns Error if querying or restoring the position fails. If restoration
         * fails, the cursor is unspecified. EOF/error indicators may be cleared.
         * External modification of the file is not synchronized by this operation.
         */
        std::uint64_t getSize();
    private:
        struct FileCloser {
            void operator()(std::FILE*);
        };

        std::unique_ptr<std::FILE, FileCloser> m_file{nullptr};
    };
}
