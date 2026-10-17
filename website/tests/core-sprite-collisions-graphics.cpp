// Runs published setup/update/render in a hidden GL window, without UI automation.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <robot2D/Graphics/GL.hpp>
#include "../public/examples/sprite-collisions/CollisionSprite.hpp"
#include <iostream>

class CollisionHarness final : public CollisionSprite {
public:
    using CollisionSprite::CollisionSprite;
    void attach(robot2D::RenderWindow& window) { m_window = &window; }
    void positionForTest() { sprite().setPosition({100.F, 180.F}); }
};

int main(int argc, char** argv) {
    if(argc != 2) return 1;
    try {
        robot2D::robot2DInit();
        robot2D::DesktopRuntime desktop;
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        robot2D::RenderWindow window(desktop, {800, 600}, "Collision lesson test");
        CollisionHarness application(argv[1]);
        application.attach(window);
        try {
            application.setup();
            application.positionForTest();
            robot2D::Event::KeyPressed press;
            press.code = robot2D::Key::D;
            application.handleEvents(robot2D::Event(press));
            application.update(2.F);

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
            application.render();
            std::array<unsigned char, 4> contact{}, wall{};
            glReadPixels(266, 600 - 1 - 134, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, contact.data());
            glReadPixels(380, 600 - 1 - 220, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, wall.data());
            const auto quads = window.getStats().drawQuads;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteTextures(1, &texture);
            glDeleteFramebuffers(1, &framebuffer);
            application.destroy();
            application.destroy();
            if(contact[0] < 220 || contact[1] < 160 || contact[2] > 90 ||
                wall[0] > 80 || wall[1] < 120 || wall[2] < 125 || quads != 9)
                throw std::runtime_error("Expected amber contact quad and teal wall after collision");
            if(glGetError() != GL_NO_ERROR) throw std::runtime_error("OpenGL error");
            std::cout << "Collision application rendered nine quads with contact feedback; cleanup passed\n";
        }
        catch(...) {
            application.destroy();
            throw;
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
