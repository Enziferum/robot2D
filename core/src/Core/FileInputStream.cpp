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

#include <filesystem>
#include <stdexcept>

#ifndef _WIN32
#include <sys/types.h>
#endif

namespace {
#ifdef _WIN32
    using FileOffset = __int64;

    int seekFile(std::FILE* file, FileOffset offset, int origin) {
        return _fseeki64(file, offset, origin);
    }

    FileOffset tellFile(std::FILE* file) {
        return _ftelli64(file);
    }
#else
    using FileOffset = off_t;

    int seekFile(std::FILE* file, FileOffset offset, int origin) {
        return ::fseeko(file, offset, origin);
    }

    FileOffset tellFile(std::FILE* file) {
        return ::ftello(file);
    }
#endif
}

namespace robot2D {

    void FileInputStream::FileCloser::operator()(std::FILE* file) {
        std::fclose(file);
    }

    bool FileInputStream::openFromFile(std::string path) {
        m_file.reset();
        if(path.empty() || path.find('\0') != std::string::npos)
            return false;

#ifdef _WIN32
        try {
            const auto nativePath = std::filesystem::u8path(path);
            m_file.reset(_wfopen(nativePath.c_str(), L"rb"));
        }
        catch(const std::filesystem::filesystem_error&) {
            return false;
        }
        catch(const std::range_error&) {
            return false;
        }
#else
        m_file.reset(std::fopen(path.c_str(), "rb"));
#endif
        return m_file != nullptr;
    }

    std::uint64_t FileInputStream::read(void* buffer, std::uint64_t size) {
        if(!m_file)
            return Error;
        if(size == 0)
            return 0;
        if(buffer == nullptr || size > std::numeric_limits<std::size_t>::max())
            return Error;

        const auto count = std::fread(buffer, 1, static_cast<std::size_t>(size), m_file.get());
        return std::ferror(m_file.get()) != 0 ? Error : static_cast<std::uint64_t>(count);
    }

    std::uint64_t FileInputStream::seek(std::uint64_t position) {
        if(!m_file || position > static_cast<std::uint64_t>(std::numeric_limits<FileOffset>::max()))
            return Error;
        if(seekFile(m_file.get(), static_cast<FileOffset>(position), SEEK_SET) != 0)
            return Error;

        std::clearerr(m_file.get());
        return tell();
    }

    std::uint64_t FileInputStream::tell() {
        if(!m_file)
            return Error;
        const auto position = tellFile(m_file.get());
        return position < 0 ? Error : static_cast<std::uint64_t>(position);
    }

    std::uint64_t FileInputStream::getSize() {
        const auto position = tell();
        if(position == Error)
            return Error;

        const auto result = seekFile(m_file.get(), 0, SEEK_END);
        const auto size = result == 0 ? tell() : Error;
        if(seek(position) == Error)
            return Error;
        return size;
    }
}
