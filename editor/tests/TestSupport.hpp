#pragma once

#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

namespace tests {
    namespace fs = std::filesystem;

    inline void check(bool value, const char* expression, const char* file, int line) {
        if(!value)
            throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": " + expression);
    }

    class Directory {
    public:
        Directory(): m_parent(fs::weakly_canonical(fs::current_path())),
            m_path(m_parent / ("fixture-" + std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count()))) {
            if(!fs::create_directory(m_path))
                throw std::runtime_error("Could not create fixture: " + m_path.u8string());
        }
        ~Directory() {
            std::error_code error;
            // Preserve failures; remove only our generated direct child of the verified parent.
            if(std::uncaught_exceptions() == 0 && fs::weakly_canonical(m_path, error).parent_path() == m_parent)
                fs::remove_all(m_path, error);
        }
        Directory(const Directory&) = delete;
        Directory& operator=(const Directory&) = delete;
        fs::path operator/(const char* name) const { return m_path / name; }
        const fs::path& path() const { return m_path; }
    private:
        fs::path m_parent;
        fs::path m_path;
    };

    inline std::string read(const fs::path& path) {
        std::ifstream input(path, std::ios::binary);
        if(!input)
            throw std::runtime_error("Could not read fixture: " + path.u8string());
        std::ostringstream contents;
        contents << input.rdbuf();
        return contents.str();
    }

    inline void write(const fs::path& path, const std::string& text, bool append = false) {
        std::ofstream output(path, std::ios::binary | (append ? std::ios::app : std::ios::trunc));
        output << text;
        output.close();
        if(!output)
            throw std::runtime_error("Could not write fixture: " + path.u8string());
    }

    template<typename Predicate>
    bool waitFor(Predicate predicate) {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while(!predicate()) {
            if(std::chrono::steady_clock::now() >= deadline)
                return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return true;
    }
}

// Evaluates in Release too; accepts expressions containing initializer-list commas.
#define CHECK(...) ::tests::check(static_cast<bool>((__VA_ARGS__)), #__VA_ARGS__, __FILE__, __LINE__)
