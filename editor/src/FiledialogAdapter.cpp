#include <tfd/tinyfiledialogs.h>

#include <editor/FiledialogAdapter.hpp>

namespace editor {
    bool FiledialogAdapter::saveFile(std::string& path, const std::string &title) {
        const char* patterns[1] = {"*.anim"};
        auto result = tinyfd_saveFileDialog(title.c_str(), nullptr,
                                            1, patterns, nullptr);
        if(!result)
            return false;

        path = result;
        return true;
    }
}