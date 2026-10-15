#include <editor/ConsoleLog.hpp>
#include <algorithm>
#include <fstream>
#include <utility>

namespace editor {

    namespace {
        constexpr std::size_t maxEntries = 10000;
        constexpr std::size_t maxBytes = 2 * 1024 * 1024;
        constexpr std::size_t maxLineBytes = 64 * 1024;
        constexpr std::size_t readBytes = 256 * 1024;
        constexpr std::size_t anchorBytes = 64;

        LogLevel readLevel(const std::string& text) {
            auto start = text.find("] [");
            if(start == std::string::npos)
                return LogLevel::Unknown;
            auto end = text.find(']', start + 3);
            auto level = text.substr(start + 3, end - (start + 3));
            if(level == "trace") return LogLevel::Trace;
            if(level == "debug") return LogLevel::Debug;
            if(level == "info") return LogLevel::Info;
            if(level == "warning" || level == "warn") return LogLevel::Warning;
            if(level == "error") return LogLevel::Error;
            if(level == "critical") return LogLevel::Critical;
            return LogLevel::Unknown;
        }
    }

    ConsoleLog::ConsoleLog(std::string path): m_path{std::move(path)} {}

    void ConsoleLog::clear() {
        m_entries.clear();
        m_bytes = 0;
        ++m_version;
    }

    void ConsoleLog::resetReader() {
        clear();
        m_offset = 0;
        m_partial.clear();
        m_anchor.clear();
        m_truncated = false;
    }

    void ConsoleLog::appendLine() {
        if(!m_partial.empty() && m_partial.back() == '\r')
            m_partial.pop_back();
        if(m_truncated)
            m_partial += " [line truncated]";
        auto level = readLevel(m_partial);
        m_bytes += m_partial.size();
        m_entries.push_back({std::move(m_partial), level});
        m_partial.clear();
        m_truncated = false;
        while(m_entries.size() > maxEntries || m_bytes > maxBytes) {
            m_bytes -= m_entries.front().text.size();
            m_entries.pop_front();
        }
        ++m_version;
    }

    void ConsoleLog::poll() {
        std::ifstream input(m_path, std::ios::binary);
        if(!input) {
            if(m_offset || !m_partial.empty())
                resetReader();
            m_status = "Log file is unavailable";
            return;
        }
        // Directory metadata can lag behind writes to an open log on Windows.
        input.seekg(0, std::ios::end);
        auto end = input.tellg();
        if(end < 0) {
            m_status = "Can't read log file";
            return;
        }
        const auto size = static_cast<std::uintmax_t>(end);
        m_status.clear();
        bool reset = size < m_offset;
        if(!reset && !m_anchor.empty()) {
            std::string anchor(m_anchor.size(), '\0');
            input.seekg(static_cast<std::streamoff>(m_offset - m_anchor.size()));
            input.read(anchor.data(), static_cast<std::streamsize>(anchor.size()));
            reset = input.gcount() != static_cast<std::streamsize>(anchor.size()) || anchor != m_anchor;
        }
        if(reset)
            resetReader();
        input.clear();
        input.seekg(static_cast<std::streamoff>(m_offset));
        std::string buffer(static_cast<std::size_t>(std::min<std::uintmax_t>(readBytes, size - m_offset)), '\0');
        if(!buffer.empty()) {
            input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
            buffer.resize(static_cast<std::size_t>(input.gcount()));
            m_offset += buffer.size();
            for(char ch: buffer) {
                if(ch == '\n')
                    appendLine();
                else if(m_partial.size() < maxLineBytes)
                    m_partial += ch;
                else
                    m_truncated = true;
            }
        }
        const auto anchorSize = static_cast<std::size_t>(std::min<std::uintmax_t>(m_offset, anchorBytes));
        m_anchor.assign(anchorSize, '\0');
        if(anchorSize) {
            input.clear();
            input.seekg(static_cast<std::streamoff>(m_offset - anchorSize));
            input.read(m_anchor.data(), static_cast<std::streamsize>(anchorSize));
            if(input.gcount() != static_cast<std::streamsize>(anchorSize))
                m_anchor.clear();
        }
    }

}
