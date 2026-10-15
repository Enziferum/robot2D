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

#include <cmath>
#include <fstream>
#include <iterator>
#include <stdexcept>

#include <editor/EditorConfig.hpp>
#include <rbini/RBIni.hpp>
#include <rbini/Utils.hpp>

namespace editor {

    namespace {
        const rbini::Value* findValue(const rbini::RBIniParser& parser,
                                      const std::string& section, const std::string& key) {
            auto foundSection = parser.getSections().find(section);
            if(foundSection == parser.getSections().end())
                return nullptr;
            auto foundValue = foundSection -> second.getValues().find(key);
            return foundValue == foundSection -> second.getValues().end() ? nullptr : &foundValue -> second;
        }

        robot2D::vec2i readVector(const rbini::Value* value, robot2D::vec2i fallback) {
            if(!value || value -> asRaw().find('x') == std::string::npos)
                return fallback;
            try {
                auto [x, y] = rbini::split_2(value -> asRaw(), 'x');
                std::size_t xSize = 0, ySize = 0;
                auto result = robot2D::vec2i{std::stoi(x, &xSize), std::stoi(y, &ySize)};
                return xSize == x.size() && ySize == y.size() ? result : fallback;
            }
            catch(const std::exception&) {
                return fallback;
            }
        }

        float readRatio(const rbini::Value* value, float fallback) {
            if(!value)
                return fallback;
            try {
                std::size_t size = 0;
                auto ratio = std::stof(value -> asRaw(), &size);
                return size == value -> asRaw().size() && std::isfinite(ratio)
                       && ratio > 0.f && ratio < 1.f ? ratio : fallback;
            }
            catch(const std::exception&) {
                return fallback;
            }
        }

        std::string decodeSettings(const std::string& encoded) {
            std::string settings;
            for(std::size_t index = 0; index < encoded.size(); ++index) {
                auto symbol = encoded[index];
                if(symbol != '\\') {
                    settings += symbol;
                    continue;
                }
                if(++index == encoded.size())
                    throw std::runtime_error("Incomplete layout escape");
                switch(encoded[index]) {
                    case '\\': settings += '\\'; break;
                    case 'n': settings += '\n'; break;
                    case 'r': settings += '\r'; break;
                    default: throw std::runtime_error("Invalid layout escape");
                }
            }
            return settings;
        }
    }

    bool EditorConfig::createDefaultConfig(const std::string& path) {
        *this = EditorConfig{};
        return saveConfig(path);
    }

    bool EditorConfig::loadConfig(const std::string& path) {
        *this = EditorConfig{};
        rbini::RBIniParser parser;
        try {
            if(!parser.loadFromFile(path) || parser.getSections().find("Editor") == parser.getSections().end())
                return false;
        }
        catch(const std::exception&) {
            return false;
        }

        cameraPos = readVector(findValue(parser, "Editor", "CameraPos"), cameraPos);
        if(auto value = findValue(parser, "Assets", "ThumbnailSize")) {
            try {
                std::size_t parsed = 0;
                auto size = std::stoi(value -> asRaw(), &parsed);
                if(parsed == value -> asRaw().size() && size >= 48 && size <= 128)
                    assetsThumbnailSize = size;
            }
            catch(const std::exception&) {}
        }
        if(auto visible = findValue(parser, "Editor", "SceneSettingsVisible"))
            panelVisibility["SceneSettingsPanel"] = visible -> asRaw() != "false";
        if(auto panels = parser.getSections().find("Panels"); panels != parser.getSections().end()) {
            for(const auto& [key, value]: panels -> second.getValues()) {
                if(value.asRaw() == "true" || value.asRaw() == "false")
                    panelVisibility[key] = value.asRaw() == "true";
            }
        }
        auto size = readVector(findValue(parser, "Editor", "CameraSize"), cameraSize);
        if(size.x > 0 && size.y > 0)
            cameraSize = size;
        leftRatio = readRatio(findValue(parser, "Layout", "LeftRatio"), leftRatio);
        rightRatio = readRatio(findValue(parser, "Layout", "RightRatio"), rightRatio);
        if(leftRatio + rightRatio >= 0.95f) {
            leftRatio = EditorConfig{}.leftRatio;
            rightRatio = EditorConfig{}.rightRatio;
        }
        centerTopRatio = readRatio(findValue(parser, "Layout", "CenterTopRatio"), centerTopRatio);
        auto created = findValue(parser, "Editor", "LayoutCreated");
        if(!created || created -> asRaw() != "true")
            return true;

        auto settings = findValue(parser, "Layout", "Settings");
        auto version = findValue(parser, "Layout", "Version");
        if(version && version -> asRaw() != "1" && version -> asRaw() != "2")
            return true;
        if(settings) {
            if(!version || version -> asRaw() != "1")
                return true;
            try {
                layoutSettings = decodeSettings(settings -> asRaw());
            }
            catch(const std::exception&) {
                layoutSettings.clear();
            }
        }
        else {
            // Older robot2D.ini files appended ImGui sections after editor fields.
            std::ifstream input(path, std::ios::binary);
            std::string line;
            bool layoutStarted = false;
            while(std::getline(input, line)) {
                if(!line.empty() && line.back() == '\r')
                    line.pop_back();
                if(line == "[ImGuiLayout]") {
                    layoutSettings.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
                    break;
                }
                if(line.rfind("[Window][", 0) == 0 || line == "[Docking][Data]")
                    layoutStarted = true;
                if(layoutStarted)
                    layoutSettings += line + '\n';
            }
        }
        layoutCreated = !layoutSettings.empty();
        return true;
    }

    bool EditorConfig::saveConfig(const std::string& path) const {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        if(!output.is_open())
            return false;
        output << "[Editor]\n"
               << "CameraPos=" << cameraPos.x << 'x' << cameraPos.y << '\n'
               << "CameraSize=" << cameraSize.x << 'x' << cameraSize.y << '\n'
               << "LayoutCreated=" << (layoutCreated ? "true" : "false") << "\n\n"
               << "[Panels]\n";
        for(const auto& [key, visible]: panelVisibility)
            output << key << '=' << (visible ? "true" : "false") << '\n';
        output << "\n[Assets]\nThumbnailSize=" << assetsThumbnailSize << "\n"
               << "\n[Layout]\n"
               << "Version=2\n"
               << "LeftRatio=" << leftRatio << '\n'
               << "RightRatio=" << rightRatio << '\n'
               << "CenterTopRatio=" << centerTopRatio << "\n\n"
               << "; Saved window positions and docking nodes.\n"
               << "[ImGuiLayout]\n"
               << layoutSettings;
        output.flush();
        return output.good();
    }
} // namespace editor
