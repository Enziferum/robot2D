#include <fstream>
#include <editor/EditorConfig.hpp>
#include <rbini/RBIni.hpp>
#include <rbini/Utils.hpp>

#include <imgui/ImGui.hpp>

namespace rbini {
    template<>
    struct value_formatter<::robot2D::vec2f> {
        static std::string write(const robot2D::vec2f& value) {
            constexpr int precision = 2;
            std::string s = to_string(value.x, precision) + 'x' + to_string(value.y, precision);
            return s;
        }

        static ::robot2D::vec2f read(const std::string& rawData) {
            auto [x, y] = split_2(rawData, 'x');
            return { std::stof(x), std::stof(y) };
        }
    };

    template<>
    struct value_formatter<::robot2D::vec2i> {
        static std::string write(const robot2D::vec2i& value) {
            constexpr int precision = 2;
            std::string s = std::to_string(value.x) + 'x' + std::to_string(value.y);
            return s;
        }

        static ::robot2D::vec2i read(const std::string& rawData) {
            auto [x, y] = split_2(rawData, 'x');
            return { std::stoi(x), std::stoi(y) };
        }
    };
}


namespace editor {

    void EditorConfig::createDefaultConfig(const std::string& path) {
        rbini::RBIniParser parser;
        rbini::Section section{};

        rbini::Value cameraPos;
        cameraPos.setAs(robot2D::vec2i{0, 0});
        section["CameraPos"] = cameraPos;
        rbini::Value cameraSize;
        cameraSize.setAs(robot2D::vec2i{1280, 720});
        section["CameraSize"] = cameraSize;
        rbini::Value boolValue;
        bool layoutCreated = false;
        boolValue.setAs(layoutCreated);
        section["LayoutCreated"] = boolValue;
        parser["Editor"] = section;

        parser.save2File(path);
    }

    void EditorConfig::loadConfig(const std::string& path) {
        auto& config = EditorConfig::getConfig();
        rbini::RBIniParser parser;
        if(!parser.loadFromFile(path, config.fieldsValue + 1))
            return;
        const auto& editorSection = parser["Editor"];
        config.cameraPos = editorSection["CameraPos"].as<robot2D::vec2i>();
        config.cameraSize = editorSection["CameraSize"].as<robot2D::vec2i>();
        config.layoutCreated = editorSection["LayoutCreated"].as<bool>();


        if(!config.layoutCreated)
            return;
        std::fstream file(path, std::ios::in);
        if(!file.is_open())
            return;
        std::string line;
        const int skipValue = config.fieldsValue + 1;
        int cntr = 0;

        std::string imguiString = "";
        while(std::getline(file, line)) {
            if(cntr < skipValue) {
                ++cntr;
                continue;
            }

            imguiString += line + "\n";
        }

        ImGui::LoadIniSettingsFromMemory(imguiString.c_str(), imguiString.length());
    }

    void EditorConfig::saveConfig(const std::string& path) {
        auto& config = EditorConfig::getConfig();
        rbini::RBIniParser parser;
        rbini::Section section{};

        rbini::Value cameraPos;
        cameraPos.setAs(robot2D::vec2i{0, 0});
        section["CameraPos"] = cameraPos;
        rbini::Value cameraSize;
        cameraSize.setAs(robot2D::vec2i{1280, 720});
        section["CameraSize"] = cameraSize;
        rbini::Value boolValue;
        boolValue.setAs(config.layoutCreated);
        section["LayoutCreated"] = boolValue;
        parser["Editor"] = section;

        parser.save2File(path);
    }

} // namespace editor