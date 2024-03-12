#pragma once
#include <string>

namespace editor {
    class FiledialogAdapter {
    public:
        static FiledialogAdapter* get() {
            static FiledialogAdapter adapter;
            return &adapter;
        }

        bool openFile(std::string& path, std::string title) { return false;  }
        bool saveFile(std::string& path, const std::string& title);
    private:
        enum class LibType {
            TinyFileDialog
        } m_libType;
    };
}