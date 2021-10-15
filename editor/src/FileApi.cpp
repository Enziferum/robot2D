/*********************************************************************
(c) Alex Raag 2021
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

#include <editor/FileApi.hpp>

namespace editor {
    namespace fs = std::filesystem;

    bool createDirectory(const std::string& path) {
        if(path.empty())
            return false;
        return fs::create_directories(fs::path(path));
    }

    bool createDirectory(const std::string& basePath, const std::string& appendPath){}

    bool deleteDirectory(const std::string& path) {
        if(path.empty())
            return false;

        auto result = fs::remove_all(fs::path(path));
        return true;
    }

    std::string addFilename(const std::string& path, const std::string& filename) {
        if(path.empty())
            return "";
        if(filename.empty())
            return "";
        fs::path resultPath = fs::path(path);
        resultPath.append(filename);
        return resultPath.string();
    }
}

