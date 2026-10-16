#define GLFW_INCLUDE_NONE
#include "TestSupport.hpp"
#include <GLFW/glfw3.h>
#include <robot2D/Graphics/GL.hpp>
#include <robot2D/imgui/Gui.hpp>
#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Graphics/RenderAPI.hpp>
#include "Render.hpp"
#include "FontSet.hpp"
#include <cmath>
#include <iostream>
#include <limits>

namespace {
    using namespace robot2D;
    struct Context {
        ImGuiContext* previous = ImGui::GetCurrentContext();
        ImGuiContext* owned = ImGui::CreateContext();
        ~Context() {
            ImGui::DestroyContext(owned); ImGui::SetCurrentContext(previous);
        }
    };

    void math() {
        for(const auto origin : {ImVec2(120, 350), ImVec2(-120, -350)}) {
            OrthoView view;
            view.update({origin.x, origin.y, 80, 40});
            const auto* matrix = view.getMatrix();
            CHECK(std::abs(matrix[0] * origin.x + matrix[12] + 1) < 0.0001f);
            CHECK(std::abs(matrix[5] * origin.y + matrix[13] - 1) < 0.0001f);
            CHECK(std::abs(matrix[0] * (origin.x + 80) + matrix[12] - 1) < 0.0001f);
            CHECK(std::abs(matrix[5] * (origin.y + 40) + matrix[13] + 1) < 0.0001f);
        }
        std::array<int, 4> clip;
        CHECK(calculateGuiClip({119, 351.25f, 130.25f, 360.25f}, {120, 350}, {2, 2}, {64, 64}, clip));
        CHECK(clip == std::array<int, 4>{0, 2, 21, 19});
        CHECK(!calculateGuiClip({0, 0, 0, 10}, {}, {1, 1}, {64, 64}, clip));
        CHECK(!calculateGuiClip({70, 0, 80, 10}, {}, {1, 1}, {64, 64}, clip));
        CHECK(!calculateGuiClip({0, 0, 10, 10}, {}, {1, 1}, {0, 64}, clip));
        CHECK(!calculateGuiClip({0, 0, 10, 10}, {}, {NAN, 1}, {64, 64}, clip));
        CHECK(calculateGuiClip({-1e30f, -1e30f, 1e30f, 1e30f}, {}, {1, 1}, {64, 64}, clip));
        CHECK(clip == std::array<int, 4>{0, 0, 64, 64});
    }

    void uninitialized() {
        Context previous;
        Gui gui;
        gui.shutdown(); gui.shutdown();
        CHECK(gui.state() == GuiState::Uninitialized);
        CHECK(!gui.beginFrame(1.f / 60)); CHECK(!gui.endFrame()); CHECK(!gui.cancelFrame()); CHECK(!gui.render());
        CHECK(!gui.initialize({}));
        CHECK(!gui.context()); CHECK(!gui.font(1));
        CHECK(ImGui::GetCurrentContext() == previous.owned);
        Texture emptyTexture;
        emptyTexture.release(); // No GL loader or context exists in this process.
    }

    void fonts() {
        {
            Context context;
            priv::FontSet fonts;
            GuiConfiguration config;
            config.defaultFont = 1;
            GuiFontConfig font;
            font.id = 1;
            const auto bytes = tests::read(std::string(RB2D_TEST_FONT_DIR) + "/notosans-regular.ttf");
            font.bytes.assign(bytes.begin(), bytes.end());
            config.fonts.push_back(font);
            // Add another base before a merge explicitly targeting the first one.
            font.id = 2; config.fonts.push_back(font);
            font.id = 3; font.mergeInto = 1; config.fonts.push_back(font);
            std::string error;
            CHECK(fonts.build(config, error));
            config.fonts.clear(); font.bytes.clear();
            CHECK(fonts.get(1)); CHECK(fonts.get(2) != fonts.get(1)); CHECK(fonts.get(3) == fonts.get(1));
            CHECK(ImGui::GetIO().FontDefault == fonts.get(1));
            unsigned char* pixels = nullptr; int width = 0, height = 0;
            ImGui::GetIO().Fonts -> GetTexDataAsRGBA32(&pixels, &width, &height);
            CHECK(pixels && width > 0 && height > 0);
        }
        for(bool fallback : {false, true}) {
            Context context;
            priv::FontSet fonts;
            GuiConfiguration config;
            config.defaultFont = 7; config.allowDefaultFallback = fallback;
            GuiFontConfig missing; missing.id = 7; missing.path = "does-not-exist.ttf";
            config.fonts.push_back(missing);
            std::string error;
            CHECK(fonts.build(config, error) == fallback);
            if(!fallback) CHECK(error.find("id=7") != std::string::npos &&
                    error.find(missing.path) != std::string::npos);
            else CHECK(fonts.get(7));
        }
    }

    void invalidFonts() {
        for(int kind = 0; kind < 4; ++kind) {
            Context context;
            priv::FontSet fonts;
            GuiConfiguration config;
            config.defaultFont = 1; config.allowDefaultFallback = true;
            GuiFontConfig font; font.id = 1; font.path = "missing.ttf";
            config.fonts = {font};
            if(kind == 0) config.fonts.push_back(font);
            if(kind == 1) config.fonts[0].mergeInto = 99;
            if(kind == 2) config.fonts[0].size = NAN;
            if(kind == 3) config.fonts[0].glyphRanges = {10, 5, 0};
            std::string error;
            CHECK(!fonts.build(config, error)); CHECK(!error.empty());
        }
    }

    GuiConfiguration configuration(Window& window) {
        GuiConfiguration config; config.window = &window; config.allowDefaultFallback = true;
        return config;
    }

    void lifecycle(Window& window) {
        Context previous;
        Gui gui;
        auto config = configuration(window);
        CHECK(gui.initialize(config)); CHECK(gui.state() == GuiState::Ready);
        CHECK(ImGui::GetCurrentContext() == previous.owned);
        CHECK(!gui.initialize(config));
        for(float dt : {0.f, -1.f, NAN, INFINITY}) {
            CHECK(!gui.beginFrame(dt)); CHECK(gui.state() == GuiState::Ready);
        }
        CHECK(gui.beginFrame(1.f / 60)); CHECK(ImGui::GetCurrentContext() == gui.context());
        int w = 0, h = 0;
        glfwGetFramebufferSize(static_cast<GLFWwindow*>(window.getRaw()), &w, &h);
        CHECK(window.getFramebufferSize() == vec2u(static_cast<unsigned>(w), static_cast<unsigned>(h)));
        CHECK(ImGui::GetIO().DisplayFramebufferScale.x == static_cast<float>(w) / window.getLogicalSize().x);
        ImGui::Begin("cancelled"); ImGui::TextUnformatted("test"); ImGui::End();
        CHECK(!gui.beginFrame(1.f / 60)); CHECK(gui.cancelFrame());
        CHECK(ImGui::GetCurrentContext() == previous.owned);
        CHECK(gui.beginFrame(1.f / 60)); CHECK(gui.endFrame());
        CHECK(!gui.beginFrame(1.f / 60));
        {
            GuiContextScope scope(gui.context());
            ImGui::GetDrawData() -> DisplaySize = {0, 0}; // Empty/minimized target consumes without GPU work.
        }
        CHECK(gui.render()); CHECK(gui.state() == GuiState::Ready); CHECK(!gui.render());
        CHECK(ImGui::GetCurrentContext() == previous.owned);
        CHECK(gui.beginFrame(1.f / 60));
        gui.shutdown(); gui.shutdown();
        CHECK(gui.state() == GuiState::Uninitialized); CHECK(ImGui::GetCurrentContext() == previous.owned);
        config.allowDefaultFallback = false; config.defaultFont = 1;
        GuiFontConfig missing; missing.id = 1; missing.path = "missing-required.ttf"; config.fonts = {missing};
        CHECK(!gui.initialize(config)); CHECK(!gui.context()); CHECK(gui.state() == GuiState::Uninitialized);
        CHECK(ImGui::GetCurrentContext() == previous.owned);
        CHECK(gui.initialize(configuration(window))); gui.shutdown();
    }

    PFNGLCREATESHADERPROC realCreateShader = nullptr;
    PFNGLCREATEPROGRAMPROC realCreateProgram = nullptr;
    PFNGLGENBUFFERSPROC realGenBuffers = nullptr;
    GLuint createdProgram = 0;
    std::vector<GLuint> createdBuffers;
    GLuint APIENTRY failFragment(GLenum type) {
        return type == GL_FRAGMENT_SHADER ? 0 : realCreateShader(type);
    }
    GLuint APIENTRY trackProgram() {
        createdProgram = realCreateProgram(); return createdProgram;
    }
    void APIENTRY trackBuffers(GLsizei count, GLuint* names) {
        realGenBuffers(count, names);
        for(GLsizei i = 0; i < count; ++i) {
            glBindBuffer(GL_ARRAY_BUFFER, names[i]);
            createdBuffers.push_back(names[i]);
        }
    }
    void rollback(Window& window) {
        Context previous;
        Gui gui;
        realCreateShader = glad_glCreateShader; realCreateProgram = glad_glCreateProgram;
        realGenBuffers = glad_glGenBuffers;
        struct DispatchRestore {
            ~DispatchRestore() {
                glad_glCreateShader = realCreateShader; glad_glCreateProgram = realCreateProgram;
                glad_glGenBuffers = realGenBuffers;
            }
        } restore;
        glad_glCreateShader = failFragment; glad_glCreateProgram = trackProgram; glad_glGenBuffers = trackBuffers;
        createdBuffers.clear(); createdProgram = 0;
        CHECK(!gui.initialize(configuration(window)));
        CHECK(gui.state() == GuiState::Uninitialized); CHECK(!gui.context());
        CHECK(ImGui::GetCurrentContext() == previous.owned);
        CHECK(gui.lastError().find("fragment") != std::string::npos);
        CHECK(createdProgram != 0 && !glIsProgram(createdProgram));
        CHECK(!createdBuffers.empty());
        for(auto buffer : createdBuffers) CHECK(!glIsBuffer(buffer));
        gui.shutdown(); gui.shutdown();
        glad_glCreateShader = realCreateShader;
        CHECK(gui.initialize(configuration(window)));
        GLuint atlas = 0;
        { GuiContextScope scope(gui.context());
          atlas = static_cast<GLuint>(reinterpret_cast<intptr_t>(ImGui::GetIO().Fonts -> TexID)); }
        CHECK(glIsTexture(atlas));
        gui.shutdown();
        CHECK(!glIsProgram(createdProgram)); CHECK(!glIsTexture(atlas));
        for(auto buffer : createdBuffers) CHECK(!glIsBuffer(buffer));
        CHECK(glGetError() == GL_NO_ERROR);
    }

    void expectPixel(int x, int y, int red, int green, int blue) {
        std::array<unsigned char, 4> pixel{};
        glReadPixels(x, 127 - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
        if(std::abs(int(pixel[0]) - red) > 2 || std::abs(int(pixel[1]) - green) > 2 ||
            std::abs(int(pixel[2]) - blue) > 2)
            throw std::runtime_error("pixel " + std::to_string(x) + "," + std::to_string(y) + " received " +
                      std::to_string(pixel[0]) + "," + std::to_string(pixel[1]) + "," + std::to_string(pixel[2]));
        CHECK(std::abs(int(pixel[1]) - green) <= 2);
        CHECK(std::abs(int(pixel[2]) - blue) <= 2);
    }

    std::vector<int> calls;
    void disturb(const ImDrawList*, const ImDrawCmd*) {
        expectPixel(8, 8, 255, 0, 0); // Earlier draw must already have executed.
        calls.push_back(1);
        glUseProgram(0); glBindVertexArray(0); glActiveTexture(GL_TEXTURE3);
        glDisable(GL_BLEND); glEnable(GL_DEPTH_TEST); glDepthMask(GL_TRUE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); glEnable(GL_RASTERIZER_DISCARD);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    void mark(const ImDrawList*, const ImDrawCmd*) {
        calls.push_back(2); expectPixel(24, 8, 0, 255, 0);
    }
    void throwing(const ImDrawList*, const ImDrawCmd*) {
        throw std::runtime_error("intentional callback failure");
    }

    struct Surface {
        GLuint fbo = 0, texture = 0;
        Surface() {
            glGenFramebuffers(1, &fbo); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glGenTextures(1, &texture); glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
            CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
            glViewport(0, 0, 128, 128); glDisable(GL_SCISSOR_TEST);
            glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT);
        }
        ~Surface() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0); glDeleteFramebuffers(1, &fbo); glDeleteTextures(1, &texture);
        }
    };

    void renderer(Window& window) {
        Context previous;
        Gui gui; CHECK(gui.initialize(configuration(window)));
        Surface surface;
        Texture image;
        const std::array<unsigned char, 4> cyan{0, 255, 255, 255};
        image.create({1, 1}, cyan.data());
        CHECK(gui.beginFrame(1.f / 60));
        auto* list = ImGui::GetBackgroundDrawList();
        list -> AddRectFilled({0, 0}, {16, 16}, IM_COL32(255, 0, 0, 255));
        list -> AddCallback(disturb, nullptr);
        list -> AddRectFilled({16, 0}, {32, 16}, IM_COL32(0, 255, 0, 255));
        list -> AddCallback(ImDrawCallback_ResetRenderState, nullptr);
        list -> AddCallback(mark, nullptr);
        list -> PushClipRect({48, 0}, {56, 16}, true);
        list -> AddRectFilled({48, 0}, {64, 16}, IM_COL32(255, 0, 255, 255));
        list -> PopClipRect();
        list -> AddImage(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(image.getID())), {64, 0}, {80, 16});
        auto* foreground = ImGui::GetForegroundDrawList();
        foreground -> AddRectFilled({32, 0}, {48, 16}, IM_COL32(0, 0, 255, 255));
        CHECK(gui.endFrame());
        {
            GuiContextScope scope(gui.context());
            auto* data = ImGui::GetDrawData();
            const ImVec2 origin(-120, 350);
            data -> DisplayPos = origin; data -> DisplaySize = {64, 64}; data -> FramebufferScale = {2, 2};
            for(int n = 0; n < data -> CmdListsCount; ++n) {
                auto* commands = data -> CmdLists[n];
                for(auto& vertex : commands -> VtxBuffer) {
                    vertex.pos.x = vertex.pos.x / 2 + origin.x; vertex.pos.y = vertex.pos.y / 2 + origin.y;
                }
                for(auto& command : commands -> CmdBuffer) {
                    command.ClipRect.x = command.ClipRect.x / 2 + origin.x;
                    command.ClipRect.y = command.ClipRect.y / 2 + origin.y;
                    command.ClipRect.z = command.ClipRect.z / 2 + origin.x;
                    command.ClipRect.w = command.ClipRect.w / 2 + origin.y;
                }
            }
        }
        ShaderHandler callerShader;
        CHECK(callerShader.createShader(ShaderType::Vertex, "#version 330 core\nvoid main(){gl_Position=vec4(0);}",
            false));
        CHECK(callerShader.createShader(ShaderType::Fragment, "#version 330 core\nout vec4 c;void main(){c=vec4(1);}",
            false));
        callerShader.use();
        GLuint vao = 0, buffer = 0, sampler = 0;
        glGenVertexArrays(1, &vao); glBindVertexArray(vao);
        glGenBuffers(1, &buffer); glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glGenSamplers(1, &sampler); glBindSampler(0, sampler); glBindSampler(3, sampler);
        glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, surface.texture);
        glViewport(1, 2, 60, 61); glScissor(2, 3, 4, 5); glEnable(GL_SCISSOR_TEST);
        glEnable(GL_DEPTH_TEST); glDepthMask(GL_TRUE); glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE);
        calls.clear(); CHECK(gui.render()); CHECK(calls == std::vector<int>{1, 2});
        GLint value = 0; glGetIntegerv(GL_CURRENT_PROGRAM, &value); CHECK(value == callerShader.getProgram());
        glGetIntegerv(GL_ACTIVE_TEXTURE, &value); CHECK(value == GL_TEXTURE3);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &value); CHECK(static_cast<GLuint>(value) == vao);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &value); CHECK(static_cast<GLuint>(value) == buffer);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &value); CHECK(static_cast<GLuint>(value) == surface.texture);
        glGetIntegerv(GL_SAMPLER_BINDING, &value); CHECK(static_cast<GLuint>(value) == sampler);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &value); CHECK(static_cast<GLuint>(value) == surface.fbo);
        std::array<GLint, 4> viewport; glGetIntegerv(GL_VIEWPORT, viewport.data());
        CHECK(viewport == std::array<GLint, 4>{1, 2, 60, 61});
        std::array<GLboolean, 4> mask; glGetBooleanv(GL_COLOR_WRITEMASK, mask.data());
        CHECK(mask == std::array<GLboolean, 4>{GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE});
        CHECK(glIsEnabled(GL_DEPTH_TEST)); CHECK(glIsEnabled(GL_SCISSOR_TEST));
        CHECK(!glIsEnabled(GL_RASTERIZER_DISCARD));
        expectPixel(40, 8, 0, 0, 255);
        expectPixel(52, 8, 255, 0, 255); expectPixel(60, 8, 0, 0, 0);
        expectPixel(72, 8, 0, 255, 255);
        CHECK(gui.beginFrame(1.f / 60)); ImGui::GetBackgroundDrawList() -> AddCallback(throwing, nullptr);
        CHECK(gui.endFrame()); CHECK(!gui.render()); CHECK(gui.state() == GuiState::Ready);
        CHECK(ImGui::GetCurrentContext() == previous.owned);
        CHECK(gui.beginFrame(1.f / 60)); CHECK(gui.endFrame()); CHECK(gui.render());
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); glDisable(GL_SCISSOR_TEST); glDisable(GL_DEPTH_TEST);
        // A non-GUI draw after the pass sees the restored target and remains usable.
        glClearColor(1, 1, 0, 1); glClear(GL_COLOR_BUFFER_BIT); expectPixel(8, 8, 255, 255, 0);
        glBindSampler(0, 0); glBindSampler(3, 0); glDeleteSamplers(1, &sampler);
        glBindVertexArray(0); glDeleteVertexArrays(1, &vao); glDeleteBuffers(1, &buffer);
        CHECK(glGetError() == GL_NO_ERROR);
        glUseProgram(0); callerShader.release();
        CHECK(glIsTexture(image.getID()));
        gui.shutdown();
    }
}

int main(int argc, char** argv) {
    try {
        CHECK(argc >= 2);
        const std::string scenario = argv[1];
        if(scenario == "math") math();
        else if(scenario == "uninitialized") uninitialized();
        else if(scenario == "fonts") {
            fonts(); invalidFonts();
        }
        else {
            if(!glfwInit()) return 77;
            const bool api45 = argc > 2 && std::string(argv[2]) == "45";
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api45 ? 5 : 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            auto* probe = glfwCreateWindow(32, 32, "GUI test probe", nullptr, nullptr);
            if(!probe) {
                glfwTerminate(); return 77;
            }
            glfwDestroyWindow(probe);
            {
                WindowContext options(false, false,
                    api45 ? WindowContext::RenderApi::OpenGL4_5 : WindowContext::RenderApi::OpenGL4_3);
                RenderAPI::getOpenGLVersion() = api45 ? RenderApi::OpenGL4_5 : RenderApi::OpenGL4_3;
                Window window({128, 128}, "GUI backend tests", options);
                if(scenario == "lifecycle") lifecycle(window);
                else if(scenario == "renderer") renderer(window);
                else if(scenario == "rollback") rollback(window);
                else CHECK(false && "unknown scenario");
                glfwDestroyWindow(static_cast<GLFWwindow*>(window.getRaw()));
            }
            glfwTerminate();
        }
        std::cout << scenario << " passed\n";
        return 0;
    }
    catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
