#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <robot2D/Graphics/GL.hpp>
#include "../public/examples/collect-items/CollectItems.hpp"
#include <iostream>
#include <vector>

class ItemsHarness final : public CollectItems {
public:
    using CollectItems::CollectItems;
    void attach(robot2D::RenderWindow& window) { m_window = &window; }
    void place(robot2D::vec2f position) { sprite().setPosition(position); }
};

int main(int argc, char** argv) {
    if(argc != 4) return 1;
    try {
        robot2D::robot2DInit();
        robot2D::DesktopRuntime desktop;
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        robot2D::RenderWindow window(desktop, {800, 600}, "Items lesson test");
        const bool missing = std::string(argv[1]) == "missing-font";
        ItemsHarness application(argv[2], missing ? std::string(argv[3]) + ".missing" : argv[3]);
        application.attach(window);
        try {
            application.setup();
            if(missing) throw std::logic_error("Missing font unexpectedly loaded");
            GLuint framebuffer = 0, texture = 0;
            glGenFramebuffers(1, &framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error("Incomplete framebuffer");
            glViewport(0, 0, 800, 600);
            const auto capture = [&]() {
                application.render();
                std::vector<unsigned char> pixels(800 * 600 * 4);
                glReadPixels(0, 0, 800, 600, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
                return pixels;
            };
            const auto pixel = [](const auto& pixels, unsigned x, unsigned y, unsigned channel) {
                return pixels[((599 - y) * 800 + x) * 4 + channel];
            };
            const auto before = capture();
            if(window.getStats().drawQuads != 14 || pixel(before, 220, 100, 0) < 240)
                throw std::runtime_error("Expected five gold item quads");
            unsigned white = 0;
            for(unsigned y = 35; y < 87; ++y)
                for(unsigned x = 40; x < 500; ++x)
                    if(pixel(before, x, y, 0) > 200 && pixel(before, x, y, 1) > 200 && pixel(before, x, y, 2) > 200) ++white;
            if(white < 80) throw std::runtime_error("ASCII font atlas did not produce visible HUD glyphs");

            application.place({220.F, 100.F}); application.update(0.F);
            application.place({100.F, 100.F}); application.update(0.F);
            const auto after = capture();
            unsigned changed = 0;
            for(unsigned y = 35; y < 61; ++y)
                for(unsigned x = 40; x < 400; ++x)
                    if(pixel(before, x, y, 0) != pixel(after, x, y, 0)) ++changed;
            if(application.round().score() != 10 || window.getStats().drawQuads != 13 ||
                pixel(after, 220, 100, 0) > 40 || changed < 10)
                throw std::runtime_error("Pickup did not disappear or update rendered score");
            robot2D::Event::KeyPressed restart;
            restart.code = robot2D::Key::R;
            application.handleEvents(robot2D::Event(restart));
            const auto reset = capture();
            if(application.round().score() != 0 || pixel(reset, 220, 100, 0) < 240)
                throw std::runtime_error("Restart did not restore pickup and score");
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteTextures(1, &texture);
            glDeleteFramebuffers(1, &framebuffer);
            application.destroy(); application.destroy();
            if(glGetError() != GL_NO_ERROR) throw std::runtime_error("OpenGL error");
            std::cout << "HUD glyphs, score change, item disappearance/reset and cleanup passed\n";
        }
        catch(const std::exception& error) {
            application.destroy(); application.destroy();
            if(!missing || std::string(error.what()).find("Cannot load font:") == std::string::npos) throw;
            std::cout << "Missing font reports its path and cleans up partial setup\n";
        }
    }
    catch(const robot2D::WindowCreationError& error) {
        std::cerr << error.what() << '\n'; return 77;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
