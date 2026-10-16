#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <editor/runtime/PlayerApplication.hpp>
#include <robot2D/Engine.hpp>
#include <robot2D/Util/Logger.hpp>
#include <iostream>
#include <vector>

int playerMain(const std::vector<std::string>& args) {
    robot2D::robot2DInit();
    try {
        auto manifest = editor::runtime::executableDirectory() / "game-launch.yaml";
        bool check = false;
        bool hidden = false;
        for(int i = 1; i < static_cast<int>(args.size()); ++i) {
            const std::string argument = args[i];
            if(argument == "--help") {
                std::cout << "Game [--manifest path] [--check] [--hidden]\n"; return 0;
            }
            if(argument == "--check") check = true;
            else if(argument == "--hidden") hidden = true;
            else if(argument == "--manifest" &&
                i + 1 < static_cast<int>(args.size())) manifest = std::filesystem::u8path(args[++i]);
            else throw std::runtime_error("Unknown/incomplete player argument: " + argument);
        }
        auto config = editor::runtime::loadLaunchConfig(manifest);
        if(check) {
            std::cout << "Player inputs validated: " << config.root.u8string() << '\n'; return 0;
        }
        if(hidden) {
            if(!glfwInit()) throw std::runtime_error("Cannot initialize hidden player window");
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        }
        robot2D::EngineConfiguration engineConfig;
        engineConfig.windowTitle = config.title; engineConfig.windowSize = config.size;
        engineConfig.windowContext = {true, config.fullscreen, robot2D::WindowContext::RenderApi::OpenGL4_5,
                                      robot2D::WindowContext::RenderDimensionType::TwoD};
        auto result = std::make_shared<editor::runtime::PlayerResult>();
        const auto engineResult = robot2D::runEngine(std::make_unique<editor::runtime::PlayerApplication>(config,
                result),
                std::move(engineConfig));
        return engineResult ? engineResult : result -> exitCode;
    }
    catch(const std::exception& error) {
        RB_CORE_ERROR("[Player] Startup failed; reason='{}'", error.what()); return 1;
    }
}

#ifdef _WIN32
int wmain(int argc, wchar_t** argv) {
    std::vector<std::string> args;
    for(int i = 0; i < argc; ++i) args.push_back(std::filesystem::path(argv[i]).u8string());
    return playerMain(args);
}
#else
int main(int argc, char** argv) {
    return playerMain(std::vector<std::string>(argv, argv + argc));
}
#endif
