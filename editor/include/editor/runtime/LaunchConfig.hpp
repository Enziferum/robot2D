#pragma once
#include <filesystem>
#include <string>
#include <robot2D/Core/Vector2.hpp>

namespace editor::runtime {
    struct LaunchConfig {
        std::filesystem::path root, scene, scripts, core, monoLib, monoEtc;
        std::string title;
        robot2D::vec2u size{1280, 720};
        bool fullscreen{false};
    };
    std::filesystem::path executableDirectory();
    LaunchConfig loadLaunchConfig(const std::filesystem::path& manifest);
}
