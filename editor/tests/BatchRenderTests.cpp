#define GLFW_INCLUDE_NONE
#include "TestSupport.hpp"
#include <GLFW/glfw3.h>
#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Graphics/QuadBatchRender.hpp>
#include <robot2D/Util/Logger.hpp>
#include <spdlog/sinks/ostream_sink.h>
#include <array>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>

namespace {
    using namespace tests;
    using namespace robot2D;

    class Target final : public RenderTarget {
    public:
        explicit Target(bool api45, bool threeD = false) : RenderTarget({64, 64},
                api45 ? WindowContext::RenderApi::OpenGL4_5
            : WindowContext::RenderApi::OpenGL4_3, threeD ? WindowContext::RenderDimensionType::ThreeD
                                                       : WindowContext::RenderDimensionType::TwoD) {
        }
        ~Target() override = default;
    };

    class Surface {
    public:
        explicit Surface(bool api45, bool threeD = false) : target(api45, threeD) {
            glGenFramebuffers(1, &framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glGenTextures(1, &color);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, color);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
            CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
            glViewport(0, 0, 64, 64);
        }
        ~Surface() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &framebuffer);
            glDeleteTextures(1, &color);
        }
        void begin() {
            target.beforeRender();
            target.clear(Color::Black);
        }
        void quad(const Color& tint, unsigned int layer = 1, float x = 0, float y = 0,
            const Texture* texture = nullptr) {
            RenderStates states;
            states.color = tint;
            states.layerID = layer;
            states.texture = texture;
            VertexData vertices = {
                {{x, y}, {0, 0}}, {{x + 8, y}, {1, 0}},
                {{x + 8, y + 8}, {1, 1}}, {{x, y + 8}, {0, 1}}
            };
            target.draw(vertices, states);
        }
        void expect(int x, int y, int red, int green, int blue) const {
            std::array<unsigned char, 4> pixel {};
            glReadPixels(x, 63 - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
            if(std::abs(int(pixel[0]) - red) > 2 || std::abs(int(pixel[1]) - green) > 2 ||
                std::abs(int(pixel[2]) - blue) > 2)
                throw std::runtime_error("Pixel " + std::to_string(x) + "," + std::to_string(y) +
                          " expected " + std::to_string(red) + "," + std::to_string(green) + "," +
                          std::to_string(blue) +
                          " but received " + std::to_string(pixel[0]) + "," + std::to_string(pixel[1]) + "," +
                          std::to_string(pixel[2]));
        }
        Target target;
    private:
        GLuint framebuffer = 0;
        GLuint color = 0;
    };

    void capacity(bool api45) {
        Surface surface(api45);
        const std::array<unsigned int, 5> markers = {0, 5000, 19999, 20000, 40000};
        const std::array<Color, 5> colors = {Color::Red, Color::Green, Color::Blue, Color::Yellow, Color::Cyan};
        for(unsigned int count: {5001u, 20000u, 20001u, 40001u}) {
            surface.begin();
            for(unsigned int i = 0; i < count; ++i) {
                auto found = std::find(markers.begin(), markers.end(), i);
                if(found != markers.end()) {
                    auto marker = static_cast<unsigned int>(found - markers.begin());
                    surface.quad(colors[marker], 1, static_cast<float>(marker * 8), 0);
                }
                else
                    surface.quad(Color::White, 1, -16, -16);
            }
            surface.target.afterRender();
            const auto stats = surface.target.getStats();
            CHECK(stats.drawQuads == count && stats.drawVertices == count * 4);
            CHECK(stats.drawCalls == (count + 19999) / 20000);
            for(unsigned int i = 0; i < markers.size(); ++i)
                if(markers[i] < count)
                    surface.expect(i * 8 + 4, 4, static_cast<int>(colors[i].red),
                        static_cast<int>(colors[i].green), static_cast<int>(colors[i].blue));
            surface.target.afterRender();
            CHECK(surface.target.getStats().drawCalls == stats.drawCalls);
        }
    }

    void textures(bool api45) {
        Surface surface(api45);
        std::vector<std::unique_ptr<Texture> > textures;
        for(int i = 0; i < 31; ++i) {
            const unsigned char data[] = {static_cast<unsigned char>(i * 7 + 20),
                                          static_cast<unsigned char>(240 - i * 5),
                                          static_cast<unsigned char>(i * 3 + 15), 255};
            auto texture = std::make_unique<Texture>();
            texture -> create({1, 1}, data);
            textures.emplace_back(std::move(texture));
        }
        for(int frame = 0; frame < 2; ++frame) {
            surface.begin();
            for(int i = 0; i < 31; ++i) {
                const int index = frame == 0 ? i : 30 - i;
                surface.quad(Color::White, 1, (i % 8) * 8, (i / 8) * 8, textures[index].get());
            }
            surface.quad(Color::White, 1, 56, 56, textures[0].get());
            surface.target.afterRender();
            CHECK(surface.target.getStats().drawQuads == 32);
            CHECK(surface.target.getStats().drawCalls == 3);
            for(int i = 0; i < 31; ++i) {
                const int index = frame == 0 ? i : 30 - i;
                surface.expect(i % 8 * 8 + 4, i / 8 * 8 + 4, index * 7 + 20, 240 - index * 5, index * 3 + 15);
            }
            surface.expect(60, 60, 20, 240, 15);
        }
    }

    void layers(bool api45) {
        Surface surface(api45);
        for(int i = 0; i < 3; ++i)
            surface.target.createLayer();
        CHECK(surface.target.getLayerCount() == 5);
        surface.begin();
        // Submit upper-layer overflow before the lower layer is even populated.
        for(unsigned int layer: {4u, 0u})
            for(int i = 0; i < 20001; ++i)
                surface.quad(layer == 4 ? Color::Green : Color::Red, layer,
                    i == 0 ? 0.f : -16.f, i == 0 ? 0.f : -16.f);
        surface.quad(Color::Yellow, 1, 16, 0);
        surface.quad(Color::Blue, 2, 24, 0);
        surface.quad(Color::Cyan, 3, 32, 0);
        surface.target.afterRender();
        surface.expect(4, 4, 0, 255, 0);
        surface.expect(20, 4, 255, 255, 0);
        surface.expect(28, 4, 0, 0, 255);
        surface.expect(36, 4, 0, 255, 255);
        CHECK(surface.target.getStats().drawQuads == 40005);
        CHECK(surface.target.getStats().drawCalls == 7);
        surface.begin();
        surface.target.afterRender();
        surface.expect(4, 4, 0, 0, 0);
        CHECK(surface.target.getStats().drawCalls == 0);
    }

    void flush(bool api45) {
        Surface surface(api45);
        surface.begin();
        surface.target.afterRender();
        CHECK(surface.target.getStats().drawCalls == 0);
        surface.quad(Color(255, 0, 0, 128));
        surface.target.flushRender(1);
        surface.expect(4, 4, 128, 0, 0);
        surface.quad(Color(0, 0, 255, 128));
        surface.target.afterRender();
        surface.expect(4, 4, 64, 0, 128);
        surface.target.flushRender(1);
        surface.target.afterRender();
        surface.expect(4, 4, 64, 0, 128);
        CHECK(surface.target.getStats().drawCalls == 2);
        CHECK(surface.target.getStats().drawQuads == 2);
        surface.begin();
        for(int i = 0; i < 20001; ++i)
            surface.quad(Color::Red, 1, i == 0 ? 0.f : -16.f, i == 0 ? 0.f : -16.f);
        surface.target.flushRender(1);
        surface.expect(4, 4, 255, 0, 0);
        surface.quad(Color::Blue, 1, 8, 0);
        surface.target.afterRender();
        surface.expect(12, 4, 0, 0, 255);
        CHECK(surface.target.getStats().drawCalls == 3);
        CHECK(surface.target.getStats().drawQuads == 20002);
    }

    void geometry(bool api45, bool threeD = false) {
        Surface surface(api45, threeD);
        surface.target.createLayer();
        QuadBatchRender<Vertex> batch;
        batch.initalize({{ElementType::Float2, "position"}, {ElementType::Float2, "uv"},
                {ElementType::Float4, "color"}});
        batch.preProcessBatching();
        for(int i = 0; i < 1000; ++i) {
            float x = i == 999 ? -0.5f : -4.f;
            for(const vec2f position: {vec2f{x, -0.5f}, vec2f{x + 1, -0.5f}, vec2f{x + 1, 0.5f}, vec2f{x, 0.5f}})
                batch.pack(Vertex{position, {0, 0}});
        }
        batch.processBatching();
        CHECK(batch.getVertexArray() -> getIndexBuffer() -> getSize() == 6000 * sizeof(uint32_t));
        ShaderHandler shader;
        CHECK(shader.createShader(ShaderType::Vertex,
            "#version 430 core\nlayout(location=0) in vec2 position; void main(){gl_Position=vec4(position,0,1);}",
            false, "batch-test.vert"));
        CHECK(shader.createShader(ShaderType::Fragment,
            "#version 430 core\nout vec4 pixel; void main(){pixel=vec4(1,0,0,1);}", false, "batch-test.frag"));
        surface.begin();
        RenderStates states;
        states.layerID = 2;
        states.shader = &shader;
        states.renderInfo.indexCount = batch.getIndexCount();
        if(threeD) {
            surface.target.setView3D(Matrix3D{}, Matrix3D{});
            surface.target.draw3D(batch.getVertexArray(), states);
        }
        else
            surface.target.draw(batch.getVertexArray(), states);
        surface.target.flushRender(2);
        surface.expect(32, 32, 255, 0, 0);
        surface.target.afterRender();
        CHECK(surface.target.getStats().drawCalls == 1);
        surface.begin();
        surface.target.afterRender();
        surface.expect(32, 32, 0, 0, 0);
        CHECK(surface.target.getStats().drawCalls == 0);
        glDeleteProgram(shader.getProgram());
    }

    // TextSystem/Core Text submit an atlas-backed VertexArray through this same path.
    class AtlasGeometry {
    public:
        AtlasGeometry(const Color& color, float x = 0) {
            const unsigned char coverage = 255;
            atlas.create({1, 1}, &coverage, 0, ImageColorFormat::RED);
            CHECK(shader.createShader(ShaderType::Vertex,
                "#version 430 core\nlayout(location=0) in vec2 position; layout(location=1) in vec2 uv;"
                "layout(location=2) in vec4 color; out vec2 coords; out vec4 tint; uniform mat4 projection;"
                "void main(){coords=uv; tint=color; gl_Position=projection*vec4(position,0,1);}",
                false, "mixed-text.vert"));
            CHECK(shader.createShader(ShaderType::Fragment,
                "#version 430 core\nin vec2 coords; in vec4 tint; out vec4 pixel; uniform sampler2D atlas;"
                "void main(){pixel=tint*texture(atlas,coords).r;}", false, "mixed-text.frag"));
            shader.use();
            shader.set("atlas", 0);
            shader.unUse();
            batch.initalize({{ElementType::Float2, "position"}, {ElementType::Float2, "uv"},
                    {ElementType::Float4, "color"}});
            batch.preProcessBatching();
            for(const vec2f pos: {vec2f{x, 0}, vec2f{x + 8, 0}, vec2f{x + 8, 8}, vec2f{x, 8}})
                batch.pack(Vertex{pos, {0, 0}, color.toGL()});
            batch.processBatching();
        }
        ~AtlasGeometry() {
            glDeleteProgram(shader.getProgram());
        }
        void draw(Target& target, unsigned int layer = 1) {
            shader.use();
            shader.setMatrix("projection", target.getView(layer).getTransform().get_matrix());
            shader.unUse();
            RenderStates states;
            states.layerID = layer;
            states.shader = &shader;
            states.texture = &atlas;
            states.renderInfo.indexCount = batch.getIndexCount();
            states.blendMode = BlendMode::MinusAlphaOne;
            target.draw(batch.getVertexArray(), states);
        }
    private:
        QuadBatchRender<Vertex> batch;
        ShaderHandler shader;
        Texture atlas;
    };

    void mixed(bool api45) {
        Surface surface(api45);
        AtlasGeometry text(Color(0, 255, 0, 128));
        AtlasGeometry geometry(Color(255, 255, 0, 128), 16);
        surface.begin();
        surface.quad(Color(255, 0, 0, 128));
        text.draw(surface.target);
        surface.quad(Color(0, 0, 255, 128));
        surface.quad(Color(255, 0, 0, 128), 1, 16);
        geometry.draw(surface.target);
        surface.quad(Color(0, 0, 255, 128), 1, 16);
        surface.target.afterRender();
        surface.expect(4, 4, 32, 64, 128);
        surface.expect(20, 4, 96, 64, 128);
        CHECK(surface.target.getStats().drawCalls == 5);
        surface.target.flushRender(1);
        surface.target.afterRender();
        surface.expect(4, 4, 32, 64, 128);
        CHECK(surface.target.getStats().drawCalls == 5);
        surface.begin();
        text.draw(surface.target);
        surface.target.afterRender();
        surface.expect(4, 4, 0, 128, 0);
        CHECK(surface.target.getStats().drawCalls == 1);
        surface.begin();
        surface.quad(Color(255, 0, 0, 128));
        text.draw(surface.target);
        text.draw(surface.target);
        surface.quad(Color(0, 0, 255, 128));
        surface.target.flushRender(1);
        surface.expect(4, 4, 16, 96, 128);
        surface.target.afterRender();
        CHECK(surface.target.getStats().drawCalls == 4);
        surface.expect(4, 4, 16, 96, 128);
    }

    void mixedOverflow(bool api45) {
        Surface surface(api45);
        surface.target.createLayer();
        AtlasGeometry text(Color(0, 255, 0, 128));
        surface.begin();
        text.draw(surface.target);
        for(int i = 0; i < 20001; ++i)
            surface.quad(i == 20000 ? Color(0, 0, 255, 128) : Color::Red, 1,
                i == 20000 ? 0.f : -16.f, i == 20000 ? 0.f : -16.f);
        // Lower layers still precede the complete mixed stream, regardless of submission order.
        surface.quad(Color::Red, 0);
        surface.quad(Color::White, 2, 16);
        surface.target.afterRender();
        surface.expect(4, 4, 63, 64, 128);
        surface.expect(20, 4, 255, 255, 255);
        CHECK(surface.target.getStats().drawCalls == 5);
        surface.begin();
        surface.target.afterRender();
        surface.expect(4, 4, 0, 0, 0);
        CHECK(surface.target.getStats().drawCalls == 0);
    }

    void camera(bool api45) {
        Surface surface(api45);
        surface.begin();
        surface.quad(Color::Red);
        View shifted(FloatRect{16, 0, 64, 64});
        surface.target.setView(shifted);
        surface.quad(Color::Blue, 1, 32);
        surface.target.afterRender();
        surface.expect(4, 4, 255, 0, 0);
        surface.expect(20, 4, 0, 0, 255);
        CHECK(surface.target.getStats().drawCalls == 2);
        surface.quad(Color::Green, 1, 48);
        surface.target.setView(shifted);
        surface.quad(Color::Yellow, 1, 56);
        surface.target.afterRender();
        surface.expect(36, 4, 0, 255, 0);
        surface.expect(44, 4, 255, 255, 0);
        CHECK(surface.target.getStats().drawCalls == 3);
        View raw(FloatRect{0, 0, 64, 64});
        surface.quad(Color::Cyan, 1, 64);
        surface.target.setRawView(const_cast<float*>(raw.getTransform().get_matrix()));
        surface.quad(Color::White, 1, 56);
        surface.target.afterRender();
        surface.expect(52, 4, 0, 255, 255);
        surface.expect(60, 4, 255, 255, 255);
        surface.begin();
        View half(FloatRect{0, 0, 64, 64});
        half.setViewport({0.5f, 0, 0.5f, 1});
        surface.target.setView(half);
        surface.quad(Color::Green);
        surface.target.afterRender();
        surface.expect(34, 4, 0, 255, 0);
        surface.expect(4, 4, 0, 0, 0);
        const auto mapped = surface.target.mapPixelToCoords({48, 32});
        CHECK(std::abs(mapped.x - 32) < 0.01f && std::abs(mapped.y - 32) < 0.01f);
        surface.begin();
        surface.target.setView(raw);
        AtlasGeometry text(Color(0, 255, 0, 128));
        surface.quad(Color::Red);
        text.draw(surface.target);
        surface.target.setView(shifted);
        // Updating this shared text shader is safe because setView completed the previous segment.
        text.draw(surface.target);
        surface.quad(Color::Blue, 1, 32);
        surface.target.afterRender();
        surface.expect(4, 4, 127, 128, 0);
        surface.expect(20, 4, 0, 0, 255);
        CHECK(surface.target.getStats().drawCalls == 4);
    }

    void viewportResize(bool api45) {
        Surface surface(api45);
        surface.begin();
        surface.quad(Color::Red);
        const View view(FloatRect{0, 0, 64, 64});
        surface.target.setViewVirtual({32, 32}, view);
        surface.quad(Color::Blue, 1, 32);
        surface.target.setViewVirtual({64, 64}, view);
        surface.quad(Color::Green, 1, 48);
        surface.target.setViewVirtual({64, 64}, view);
        surface.quad(Color::Yellow, 1, 56);
        surface.target.afterRender();
        surface.expect(4, 4, 255, 0, 0);
        surface.expect(18, 40, 0, 0, 255);
        surface.expect(52, 16, 0, 255, 0);
        surface.expect(60, 16, 255, 255, 0);
        CHECK(surface.target.getStats().drawCalls == 3);
        const auto middle = surface.target.mapPixelToCoords({32, 32});
        CHECK(std::abs(middle.x - 32) < 0.01f && std::abs(middle.y - 32) < 0.01f);
        surface.target.setViewVirtual({32, 64}, view);
        // Letterbox is 32x18 at GL y=23: top-left pixel (0,23) maps to world origin.
        const auto corner = surface.target.mapPixelToCoords({0, 23});
        CHECK(std::abs(corner.x) < 0.01f && std::abs(corner.y) < 0.01f);
        const auto center = surface.target.mapPixelToCoords({16, 32});
        CHECK(std::abs(center.x - 32) < 0.01f && std::abs(center.y - 32) < 0.01f);
        GLint before[4];
        glGetIntegerv(GL_VIEWPORT, before);
        surface.target.setViewVirtual({0, 0}, view);
        GLint after[4];
        glGetIntegerv(GL_VIEWPORT, after);
        CHECK(std::equal(before, before + 4, after));
        surface.target.setViewVirtual({1, 1}, view, 999);
        const auto tiny = surface.target.mapPixelToCoords({0, 0});
        CHECK(std::isfinite(tiny.x) && std::isfinite(tiny.y));
        surface.target.setViewVirtual({64, 64}, view);
        {
            Target other(api45);
            other.setViewVirtual({32, 32}, view);
            surface.target.clearScissor({0, 0, 64, 64}, Color::Red);
            surface.expect(60, 32, 255, 0, 0);
        }
    }

    class ResizeWindow : public RenderWindow {
    public:
        using RenderWindow::RenderWindow;
        void notifyResize(int width, int height) {
            onResize(width, height);
        }
    };

    void windowResize(bool api45) {
        auto* original = glfwGetCurrentContext();
        GLFWwindow* created = nullptr;
        {
            ResizeWindow window({64, 64}, "hidden resize regression",
                WindowContext(false, false, api45 ? WindowContext::RenderApi::OpenGL4_5
                                                 : WindowContext::RenderApi::OpenGL4_3));
            created = glfwGetCurrentContext();
            CHECK(glfwGetWindowAttrib(created, GLFW_VISIBLE) == GLFW_FALSE);
            window.beforeRender();
            window.clear(Color::Black);
            RenderStates red;
            red.color = Color::Red;
            window.draw(VertexData{{{0, 0}, {0, 0}}, {{8, 0}, {1, 0}},
                    {{8, 8}, {1, 1}}, {{0, 8}, {0, 1}}}, red);
            // GLFW may update the GL viewport before delivering the logical resize event.
            glViewport(0, 0, 128, 64);
            window.resize({128, 64});
            std::array<unsigned char, 4> oldPixel {};
            glReadPixels(4, 59, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, oldPixel.data());
            CHECK(oldPixel[0] > 250 && oldPixel[1] < 3 && oldPixel[2] < 3);
            CHECK(window.getSize() == vec2u{128, 64});
            View fixed(FloatRect{0, 0, 64, 64});
            const auto point = window.mapPixelToCoords({64, 32}, fixed);
            CHECK(std::abs(point.x - 32) < 0.01f && std::abs(point.y - 32) < 0.01f);
            CHECK(window.getDefaultView().getSize() == vec2f{128, 64});
            window.setView(window.getDefaultView());
            window.notifyResize(32, 64);
            CHECK(window.getView().getSize() == vec2f{128, 64});
            CHECK(window.getView(0).getSize() == vec2f{32, 64});
            CHECK(window.getSize() == vec2u{32, 64});
            const auto next = window.mapPixelToCoords({16, 32}, fixed);
            CHECK(std::abs(next.x - 32) < 0.01f && std::abs(next.y - 32) < 0.01f);
            CHECK(window.getDefaultView().getSize() == vec2f{32, 64});
            glViewport(0, 0, 0, 0);
            window.notifyResize(0, 0);
            window.beforeRender();
            const auto minimized = window.mapPixelToCoords({16, 32}, fixed);
            CHECK(std::abs(minimized.x - 32) < 0.01f && std::abs(minimized.y - 32) < 0.01f);
            CHECK(window.getDefaultView().getSize() == vec2f{32, 64});
            window.afterRender();
            CHECK(glGetError() == GL_NO_ERROR);
        }
        glfwMakeContextCurrent(original);
        glfwDestroyWindow(created);
    }

    void malformed(bool api45, std::ostringstream& errors) {
        Surface surface(api45);
        surface.begin();
        for(unsigned int size: {0u, 3u, 5u}) {
            surface.target.draw(Vertex3DData(size), RenderStates{});
            CHECK(errors.str().find("quad rejected") != std::string::npos);
            errors.str(""); errors.clear();
        }
        surface.target.flushRender(999);
        CHECK(errors.str().find("flush rejected") != std::string::npos);
        errors.str(""); errors.clear();
        surface.target.afterRender();
        CHECK(surface.target.getStats().drawQuads == 0 && surface.target.getStats().drawCalls == 0);
        surface.expect(4, 4, 0, 0, 0);
        surface.quad(Color::Green);
        surface.target.afterRender();
        surface.expect(4, 4, 0, 255, 0);
    }
}

int main(int argc, char** argv) {
    if(argc != 3) return 1;
    const bool api45 = std::string(argv[2]) == "45";
    if(!glfwInit()) return 77;
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api45 ? 5 : 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    auto* window = glfwCreateWindow(64, 64, "Batch render tests", nullptr, nullptr);
    if(!window) {
        glfwTerminate(); return 77;
    }
    glfwMakeContextCurrent(window);
    int result = 0;
    std::ostringstream errors;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(errors);
    logger::Log::getCoreLogger() = std::make_shared<spdlog::logger>("batch-test", sink);
    logger::Log::getCoreLogger() -> set_level(spdlog::level::err);
    if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) result = 77;
    else {
        try {
            const std::string name = argv[1];
            if(name == "capacity") capacity(api45);
            else if(name == "textures") textures(api45);
            else if(name == "layers") layers(api45);
            else if(name == "flush") flush(api45);
            else if(name == "geometry") geometry(api45);
            else if(name == "three-d") geometry(api45, true);
            else if(name == "mixed") mixed(api45);
            else if(name == "mixed-overflow") mixedOverflow(api45);
            else if(name == "camera") camera(api45);
            else if(name == "viewport-resize") viewportResize(api45);
            else if(name == "window-resize") windowResize(api45);
            else if(name == "malformed") malformed(api45, errors);
            else CHECK(false && "Unknown test case");
            CHECK(glGetError() == GL_NO_ERROR);
            CHECK(errors.str().empty());
            std::cout << name << " OpenGL " << argv[2] << " passed\n";
        }
        catch(const std::exception& error) { std::cerr << error.what() << '\n' << errors.str(); result = 1; }
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return result;
}
