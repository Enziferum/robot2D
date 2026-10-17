#define GLFW_INCLUDE_NONE
#include "TestSupport.hpp"
#include <GLFW/glfw3.h>
#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Engine.hpp>
#include <robot2D/Util/Logger.hpp>
#include <editor/runtime/PlayerApplication.hpp>
#include <array>
#include <iostream>

namespace {
    class PresentationProbe final : public editor::runtime::PlayerApplication {
    public:
        PresentationProbe(editor::runtime::LaunchConfig config, std::shared_ptr<editor::runtime::PlayerResult> result,
            std::shared_ptr<bool> passed) : PlayerApplication(std::move(config), std::move(result)), m_passed(passed) {
        }
        void update(float dt) override {
            PlayerApplication::update(dt); m_updated = true;
        }
        void render() override {
            if(!m_updated || !m_running) return;
            PlayerApplication::render();
            glFinish();
            glReadBuffer(GL_FRONT);
            std::array<unsigned char, 4> wall{}, interior{};
            glReadPixels(8, 180, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, wall.data());
            glReadPixels(320, 180, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, interior.data());
            std::cout << "Presented wall=" << int(wall[0]) << ',' << int(wall[1]) << ',' << int(wall[2])
                      << "; interior=" << int(interior[0]) << ',' << int(interior[1]) << ',' << int(interior[2]) <<
                '\n';
            *m_passed = wall[0] < 3 && wall[1] < 3 && wall[2] > 250
                && interior[0] < 3 && interior[1] < 3 && interior[2] < 3;
            m_running = false;
        }
    private:
        std::shared_ptr<bool> m_passed;
        bool m_updated{false};
    };
}
int main() {
    logger::Log::Init();
    try {
        auto config = editor::runtime::loadLaunchConfig(editor::runtime::executableDirectory() / "game-launch.yaml");
        CHECK(glfwInit());
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        robot2D::EngineConfiguration engineConfig;
        engineConfig.windowSize = config.size; engineConfig.windowTitle = config.title;
        engineConfig.windowContext.renderApi = robot2D::WindowContext::RenderApi::OpenGL4_5;
        auto result = std::make_shared<editor::runtime::PlayerResult>();
        auto passed = std::make_shared<bool>(false);
        CHECK(robot2D::runEngine(std::make_unique<PresentationProbe>(config, result, passed),
            std::move(engineConfig)) == 0);
        CHECK(result->exitCode == 0);
        CHECK(*passed);
        return 0;
    }
    catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
