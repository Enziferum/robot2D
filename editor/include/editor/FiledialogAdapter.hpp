#pragma once
#include <string>
#include <vector>

namespace editor {
    class FiledialogAdapter {
    public:
        static FiledialogAdapter* get() {
            static FiledialogAdapter adapter;
            return &adapter;
        }

        bool openFile(std::string& path,
                      std::string title,
                      std::string defaultPath2Open = "",
                      const std::vector<std::string>& filePatterns = {},
                      std::string singleFilterDescription = "",
                      bool allowMultiSelection = false);
        bool saveFile(std::string& path,
                      const std::string& title,
                      std::string defaultPath2Open = "",
                      const std::vector<std::string>& filePatterns = {},
                      std::string singleFilterDescription = "");
        bool selectFolder(std::string& path,
                          const std::string& title,
                          std::string defaultPath2Open = "");
    private:
        enum class LibType {
            TinyFileDialog,
            Pfd
        } m_libType = LibType::TinyFileDialog;
    };
}