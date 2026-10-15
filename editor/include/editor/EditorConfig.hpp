#pragma once

#include <string>
#include <map>
#include <robot2D/Core/Vector2.hpp>

namespace editor {

    class EditorConfig {
    public:
        static EditorConfig& getConfig() {
            static EditorConfig config;
            return config;
        }

        bool createDefaultConfig(const std::string& path);

        bool loadConfig(const std::string& path);

        bool saveConfig(const std::string& path) const;

        robot2D::vec2i cameraPos {0, 0};
        robot2D::vec2i cameraSize {1280, 720};
        bool layoutCreated { false };
        std::map<std::string, bool> panelVisibility;
        int assetsThumbnailSize {64};
        float leftRatio {0.11f};
        float rightRatio {0.12f};
        float centerTopRatio {0.8f};
        std::string layoutSettings;
    };

} // namespace editor
