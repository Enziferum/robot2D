#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <robot2D/Graphics/GL.hpp>
#include "../public/examples/game-states/StatefulGame.hpp"
#include <iostream>
#include <vector>

class StatesHarness final : public StatefulGame {
public:
    using StatefulGame::StatefulGame;
    void attach(robot2D::RenderWindow& window) { m_window = &window; }
    void place(robot2D::vec2f position) { sprite().setPosition(position); }
};

int main(int argc, char** argv) {
    if(argc != 3) return 1;
    try {
        robot2D::robot2DInit();
        robot2D::DesktopRuntime desktop;
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        robot2D::RenderWindow window(desktop, {800, 600}, "States lesson test");
        StatesHarness game(argv[1], argv[2]); game.attach(window);
        try {
            game.setup();
            GLuint framebuffer = 0, texture = 0;
            glGenFramebuffers(1, &framebuffer); glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glGenTextures(1, &texture); glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error("Incomplete framebuffer");
            glViewport(0, 0, 800, 600);
            const auto capture = [&]() {
                game.render();
                std::vector<unsigned char> pixels(800 * 600 * 4);
                glReadPixels(0, 0, 800, 600, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
                return pixels;
            };
            const auto press = [&](robot2D::Key code) {
                robot2D::Event::KeyPressed event; event.code = code;
                game.handleEvents(robot2D::Event(event));
            };
            const auto playing = capture();
            press(robot2D::Key::P); game.update(30.F);
            const auto paused = capture();
            unsigned hudChanges = 0, worldChanges = 0;
            for(unsigned y = 0; y < 600; ++y) {
                for(unsigned x = 0; x < 800; ++x) {
                    const auto i = ((599 - y) * 800 + x) * 4;
                    if(playing[i] != paused[i]) { if(y >= 100) ++worldChanges; else ++hudChanges; }
                }
            }
            press(robot2D::Key::D); game.update(30.F);
            const auto stillPaused = capture();
            if(game.phase() != GamePhase::Paused || hudChanges < 10 || worldChanges != 0 || paused != stillPaused)
                throw std::runtime_error("Pause did not render new HUD over a frozen world");
            press(robot2D::Key::P);
            for(auto position : {robot2D::vec2f{220.F, 100.F}, {650.F, 120.F},
                    {90.F, 450.F}, {300.F, 500.F}, {650.F, 500.F}}) {
                game.place(position); game.update(0.F);
            }
            const auto won = capture();
            if(game.phase() != GamePhase::Won || game.round().score() != 50 || window.getStats().drawQuads != 9)
                throw std::runtime_error("Victory did not render final score with no items");
            press(robot2D::Key::P); game.update(30.F);
            if(capture() != won) throw std::runtime_error("P changed frozen victory frame");
            press(robot2D::Key::R); capture();
            if(game.phase() != GamePhase::Playing || game.round().score() != 0 || window.getStats().drawQuads != 14)
                throw std::runtime_error("Restart did not restore Playing scene");
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteTextures(1, &texture); glDeleteFramebuffers(1, &framebuffer);
            game.destroy(); game.destroy();
            if(glGetError() != GL_NO_ERROR) throw std::runtime_error("OpenGL error");
            std::cout << "Playing/pause/victory/restart rendering and cleanup passed\n";
        }
        catch(...) { game.destroy(); throw; }
    }
    catch(const robot2D::WindowCreationError& error) { std::cerr << error.what() << '\n'; return 77; }
    catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
