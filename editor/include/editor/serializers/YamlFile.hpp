/*********************************************************************
(c) Alex Raag 2026
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

#include <filesystem>
#include <fstream>
#include <string>
#include <yaml-cpp/yaml.h>

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#endif

namespace editor::yaml {

    inline bool writeFile(const std::string& path, const YAML::Emitter& out) {
        if(!out.good() || path.empty() || path.find('\0') != std::string::npos)
            return false;
        namespace fs = std::filesystem;
        const auto destination = fs::u8path(path);
        fs::path temporary;
        std::error_code error;
        constexpr unsigned maxAttempts = 128;
        bool available = false;
        for(unsigned i = 0; i < maxAttempts; ++i) {
            temporary = destination;
            temporary += i == 0 ? ".tmp" : ".tmp." + std::to_string(i);
            const bool exists = fs::exists(temporary, error);
            if(error)
                return false;
            if(!exists) {
                available = true;
                break;
            }
        }
        if(!available)
            return false;
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if(!output.is_open())
            return false;
        output << out.c_str() << '\n';
        output.flush();
        output.close();
        if(output) {
#ifdef _WIN32
            if(MoveFileExW(temporary.c_str(), destination.c_str(),
                           MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                return true;
#else
            fs::rename(temporary, destination, error);
            if(!error)
                return true;
#endif
        }
        fs::remove(temporary, error);
        return false;
    }
}
