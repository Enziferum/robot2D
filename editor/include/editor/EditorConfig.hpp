#pragma once

#include <string>
#include <robot2D/Core/Vector2.hpp>

namespace editor {

    class EditorConfig {
    public:
        static EditorConfig& getConfig() {
            static EditorConfig config;
            return config;
        }

        void createDefaultConfig(const std::string& path);

        void loadConfig(const std::string& path);

        void saveConfig(const std::string& path);

        robot2D::vec2i cameraPos;
        robot2D::vec2i cameraSize;
        bool layoutCreated { false };
        const int fieldsValue = 3;
    };

} // namespace editor