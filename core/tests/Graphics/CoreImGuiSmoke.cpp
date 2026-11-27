#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Graphics/Sprite.hpp>
#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/FrameBuffer.hpp>
#include <robot2D/Graphics/RenderAPI.hpp>
#include <robot2D/imgui/Gui.hpp>
#include <iostream>
#include <stdexcept>

namespace {
    void require(bool value, const char* message) {
        if(!value) throw std::runtime_error(message);
    }
}

int main() {
    try {
        robot2D::DesktopRuntime desktop;
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        robot2D::WindowContext options(false, false, robot2D::WindowContext::RenderApi::OpenGL4_1);
        {
            robot2D::RenderAPI::getOpenGLVersion() = robot2D::RenderApi::OpenGL4_5;
            robot2D::Window plainWindow(desktop, {64, 64}, "Core resources", options);
            require(robot2D::RenderAPI::getOpenGLVersion() == robot2D::RenderApi::OpenGL4_1, "Window API selection");
            glad_glTexStorage2D = nullptr;
            robot2D::FrameBufferSpecification spec;
            spec.size = {32, 32};
            spec.attachments = {robot2D::FrameBufferTextureFormat::RGBA8,
                                robot2D::FrameBufferTextureFormat::DEPTH24STENCIL8};
            auto framebuffer = robot2D::FrameBuffer::Create(spec);
            framebuffer -> Bind();
            require(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Window resources");
            framebuffer -> unBind();
        }
        robot2D::RenderWindow window(desktop, {320, 240}, "Core ImGui 4.1", options);
        robot2D::WindowMetrics fractional{{80, 60}, {120, 90}, {2.f, 2.f}, false};
        require(window.synchronizeSize(fractional), "Fractional surface");
        require(window.getDefaultView().getSize() == robot2D::vec2f(80, 60), "Logical default view");
        GLint fractionalViewport[4] = {};
        glGetIntegerv(GL_VIEWPORT, fractionalViewport);
        require(fractionalViewport[2] == 120 && fractionalViewport[3] == 90, "Physical fractional viewport");
        fractional.minimized = true;
        require(!window.synchronizeSize(fractional), "Minimized surface");
        require(window.synchronizeSize(), "Restore native surface");
        GLint major = 0, minor = 0, profile = 0, flags = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        require(major > 4 || (major == 4 && minor >= 1), "OpenGL version");
        require((profile & GL_CONTEXT_CORE_PROFILE_BIT) != 0, "Core profile");
        require((flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) != 0, "Forward compatible context");
        glad_glCreateTextures = nullptr;
        glad_glTextureStorage2D = nullptr;
        glad_glTextureSubImage2D = nullptr;
        glad_glTextureParameteri = nullptr;
        glad_glBindTextureUnit = nullptr;
        glad_glCreateFramebuffers = nullptr;
        glad_glTexStorage2D = nullptr;
        robot2D::Texture texture;
        const unsigned char pixels[] = {255, 0, 0, 255};
        texture.create({1, 1}, pixels, 0, robot2D::ImageColorFormat::RGBA);
        robot2D::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setSize({32, 32});
        for(unsigned samples : {1U, 4U}) {
            robot2D::FrameBufferSpecification spec;
            spec.size = {64, 64};
            spec.Samples = samples;
            spec.attachments = {robot2D::FrameBufferTextureFormat::RGBA8,
                                robot2D::FrameBufferTextureFormat::DEPTH24STENCIL8};
            auto framebuffer = robot2D::FrameBuffer::Create(spec);
            framebuffer -> Bind();
            require(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer");
            framebuffer -> Resize({96, 48});
            framebuffer -> Bind();
            require(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Resized framebuffer");
            framebuffer -> unBind();
        }
        robot2D::Gui gui;
        robot2D::GuiConfiguration config;
        config.window = &window;
        robot2D::GuiFontConfig font;
        font.id = 1;
        font.path = RB2D_SMOKE_FONT;
        font.glyphRanges = {0x20, 0xFF, 0x400, 0x52F, 0};
        config.fonts.push_back(font);
        config.defaultFont = 1;
        require(gui.initialize(config), "GUI initialize");
        require(gui.font(1) -> FindGlyphNoFallback(0x416) != nullptr, "Cyrillic font");
        auto* native = static_cast<GLFWwindow*>(window.getRaw());
        glfwSetWindowSize(native, 480, 300);
        desktop.processEvents();
        int width = 0, height = 0;
        glfwGetFramebufferSize(native, &width, &height);
        require(window.getFramebufferSize() == robot2D::vec2u(width, height), "Framebuffer metrics");
        require(gui.beginFrame(1.f / 60), "GUI begin");
        require(ImGui::GetIO().DisplayFramebufferScale.x ==
            static_cast<float>(width) / window.getLogicalSize().x, "Framebuffer scale");
        ImGui::Begin("Core + ImGui");
        ImGui::TextUnformatted(u8"Привет, мир");
        ImGui::End();
        require(gui.endFrame(), "GUI end");
        robot2D::FrameBufferSpecification pixelSpec;
        pixelSpec.size = {64, 64};
        pixelSpec.attachments = {robot2D::FrameBufferTextureFormat::RGBA8,
                                 robot2D::FrameBufferTextureFormat::DEPTH24STENCIL8};
        auto pixelTarget = robot2D::FrameBuffer::Create(pixelSpec);
        pixelTarget -> Bind();
        window.resize({64, 64});
        window.clear();
        window.beforeRender();
        window.draw(sprite);
        window.afterRender();
        unsigned char rendered[4] = {};
        glReadPixels(16, 48, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rendered);
        require(rendered[0] > 240 && rendered[1] < 10 && rendered[2] < 10, "Core sprite pixels");
        pixelTarget -> unBind();
        require(window.synchronizeSize(), "Native frame synchronization");
        window.clear();
        window.beforeRender();
        window.draw(sprite);
        window.afterRender();
        GLint viewport[4] = {};
        glGetIntegerv(GL_VIEWPORT, viewport);
        require(viewport[2] == width && viewport[3] == height, "Core framebuffer viewport");
        require(gui.render(), "GUI render");
        require(glGetError() == GL_NO_ERROR, "OpenGL error");
        gui.shutdown();
        require(gui.state() == robot2D::GuiState::Uninitialized, "GUI shutdown");
        std::cout << "Core + ImGui OpenGL 4.1 smoke passed\n";
        return 0;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
