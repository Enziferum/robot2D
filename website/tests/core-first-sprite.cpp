// Exercises the published lesson source, without desktop UI automation.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <robot2D/Graphics/GL.hpp>
#include "../public/examples/first-sprite/FirstSprite.hpp"
#include <algorithm>
#include <iostream>
#include <vector>

class LessonHarness final : public FirstSprite {
public:
    using FirstSprite::FirstSprite;
    void attach(robot2D::RenderWindow& window) { m_window = &window; }
};

int main(int argc, char** argv) {
    if(argc != 3) return 1;
    try {
        robot2D::robot2DInit();
        robot2D::DesktopRuntime desktop;
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        robot2D::RenderWindow window(desktop, {800, 600}, "Core lesson test");
        const bool missing = std::string(argv[1]) == "missing-texture";
        LessonHarness application(missing ? std::string(argv[2]) + ".missing" : argv[2]);
        application.attach(window);
        try {
            application.setup();
            if(missing) throw std::logic_error("Missing texture unexpectedly loaded");

            // Capture the lesson's actual render() into an FBO; display() does not
            // change the binding. No front-buffer/window screenshot is needed.
            GLuint framebuffer = 0, color = 0;
            glGenFramebuffers(1, &framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glGenTextures(1, &color);
            glBindTexture(GL_TEXTURE_2D, color);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error("Incomplete test framebuffer");
            glViewport(0, 0, 800, 600);
            application.render();
            std::vector<unsigned char> pixels(800 * 600 * 4);
            glReadPixels(0, 0, 800, 600, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            unsigned yellow = 0, outside = 0;
            for(unsigned y = 0; y < 600; ++y) {
                for(unsigned x = 0; x < 800; ++x) {
                    const auto i = (y * 800 + x) * 4;
                    if(pixels[i] > 180 && pixels[i + 1] > 100 && pixels[i + 2] < 100) {
                        ++yellow;
                        if(x < 272 || x >= 528 || y < 172 || y >= 428) ++outside;
                    }
                }
            }
            const auto stats = window.getStats();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteTextures(1, &color);
            glDeleteFramebuffers(1, &framebuffer);
            if(yellow < 100 || outside != 0 || stats.drawQuads != 1 || stats.drawCalls != 1)
                throw std::runtime_error("Expected one visible yellow Robot Lab sprite in its rectangle");
            application.handleEvents(robot2D::Event(robot2D::Event::Closed{}));
            if(application.isRunning()) throw std::runtime_error("Close event did not stop application");
            application.destroy();
            application.destroy(); // Empty release is safe.
            if(glGetError() != GL_NO_ERROR) throw std::runtime_error("OpenGL error");
            std::cout << "Rendered Robot Lab sprite: " << yellow << " yellow pixels; close and release passed\n";
        }
        catch(const std::runtime_error& error) {
            application.destroy();
            if(!missing || std::string(error.what()).find("Cannot load texture:") == std::string::npos)
                throw;
            std::cout << "Missing texture reports its absolute path; cleanup passed\n";
        }
    }
    catch(const robot2D::WindowCreationError& error) {
        std::cerr << error.what() << '\n';
        return 77;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
