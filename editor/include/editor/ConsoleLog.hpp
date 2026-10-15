#pragma once

#include <cstdint>
#include <deque>
#include <string>

namespace editor {

    enum class LogLevel { Trace, Debug, Info, Warning, Error, Critical, Unknown };

    struct ConsoleEntry {
        std::string text;
        LogLevel level;
    };

    class ConsoleLog {
    public:
        explicit ConsoleLog(std::string path);
        ConsoleLog(const ConsoleLog& other) = delete;
        ConsoleLog& operator=(const ConsoleLog& other) = delete;
        ConsoleLog(ConsoleLog&& other) = delete;
        ConsoleLog& operator=(ConsoleLog&& other) = delete;
        ~ConsoleLog() = default;

        void poll();
        void clear();
        const std::deque<ConsoleEntry>& getEntries() const { return m_entries; }
        const std::string& getPath() const { return m_path; }
        const std::string& getStatus() const { return m_status; }
        std::uint64_t getVersion() const { return m_version; }
    private:
        void resetReader();
        void appendLine();
        std::string m_path;
        std::string m_status;
        std::string m_partial;
        std::string m_anchor;
        std::deque<ConsoleEntry> m_entries;
        std::uintmax_t m_offset {0};
        std::size_t m_bytes {0};
        std::uint64_t m_version {0};
        bool m_truncated {false};
    };

}
