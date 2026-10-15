#define GLFW_INCLUDE_NONE
#include "TestSupport.hpp"
#include <GLFW/glfw3.h>
#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Graphics/Buffer.hpp>
#include <robot2D/Util/Logger.hpp>
#include <spdlog/sinks/ostream_sink.h>
#include <iostream>

namespace {
    using namespace tests;

    class Capture {
    public:
        Capture() : previous(logger::Log::getCoreLogger()) {
            auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(output);
            logger::Log::getCoreLogger() = std::make_shared<spdlog::logger>("GL-test", sink);
            logger::Log::getCoreLogger() -> set_level(spdlog::level::trace);
        }
        ~Capture() {
            logger::Log::getCoreLogger() = previous;
        }
        std::string text() const {
            return output.str();
        }
        void clear() {
            output.str(""); output.clear();
        }
    private:
        std::ostringstream output;
        std::shared_ptr<spdlog::logger> previous;
    };

    void diagnostics() {
        Capture log;
        robot2D::GLDiagnosticsOptions options;
        options.debugOutput = false;
        robot2D::configureGLDiagnostics(options);
        robot2D::logGLContextInfo();
        CHECK(log.text().find("vendor='") != std::string::npos);
        CHECK(log.text().find("SomeVersion") == std::string::npos);

        log.clear();
        CHECK(!glCall(glEnable, 0xFFFFFFFFu));
        CHECK(log.text().find("glEnable") != std::string::npos);
        CHECK(log.text().find("GL_INVALID_ENUM") != std::string::npos);
        CHECK(log.text().find("GraphicsDiagnosticsTests.cpp:") != std::string::npos);
        CHECK(glGetError() == GL_NO_ERROR);

        log.clear();
        glEnable(0xFFFFFFFFu);
        CHECK(glCall(glEnable, GL_BLEND));
        CHECK(log.text().find("origin unknown") != std::string::npos);

        options.debugOutput = true;
        options.repeatLimit = 2;
        robot2D::configureGLDiagnostics(options);
        log.clear();
        for(int i = 0; i < 4; ++i)
            glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 4242,
                GL_DEBUG_SEVERITY_HIGH, -1, "test driver message {literal}");
        auto text = log.text();
        CHECK(text.find("source=application; type=error; severity=high; id=4242") != std::string::npos);
        CHECK(text.find("{literal}") != std::string::npos);
        auto first = text.find("test driver message");
        auto second = text.find("test driver message", first + 1);
        CHECK(second != std::string::npos);
        CHECK(text.find("test driver message", second + 1) == std::string::npos);
        CHECK(text.find("suppressed") != std::string::npos);

        log.clear();
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 4243,
            GL_DEBUG_SEVERITY_NOTIFICATION, -1, "hidden notification");
        CHECK(log.text().find("hidden notification") == std::string::npos);
        options.notifications = true;
        robot2D::configureGLDiagnostics(options);
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 4243,
            GL_DEBUG_SEVERITY_NOTIFICATION, -1, "visible notification");
        CHECK(log.text().find("visible notification") != std::string::npos);

        log.clear();
        glCall(glDebugMessageInsert, GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_PERFORMANCE, 131218,
            GL_DEBUG_SEVERITY_MEDIUM, -1, "test shader state performance warning");
        CHECK(log.text().find("type=performance; severity=medium; id=131218") != std::string::npos);
        CHECK(log.text().find("operation=glDebugMessageInsert") != std::string::npos);
        CHECK(log.text().find("GraphicsDiagnosticsTests.cpp:") != std::string::npos);
        CHECK(log.text().find("test shader state performance warning") != std::string::npos);

        log.clear();
        auto buffer = robot2D::VertexBuffer::Create(16);
        float data[8] = {};
        buffer -> setData(data, sizeof(data));
        CHECK(log.text().find("requested=32 bytes; capacity=16 bytes") != std::string::npos);
        CHECK(glGetError() == GL_NO_ERROR);
        buffer -> setData(data, 16);
        CHECK(glGetError() == GL_NO_ERROR);
    }

    void shaders() {
        Capture log;
        Directory fixture;
        robot2D::ShaderHandler shader;
        CHECK(!shader.createShader(robot2D::ShaderType::Vertex, (fixture / "missing.vert").string(), true));
        CHECK(shader.getLastError().find("could not open shader file") != std::string::npos);
        CHECK(shader.getProgram() == -1);
        CHECK(!shader.createShader(robot2D::ShaderType::Vertex, "", false, "empty.vert"));
        CHECK(shader.getLastError().find("source is empty") != std::string::npos);
        CHECK(!shader.createShader(robot2D::ShaderType::Vertex,
            "#version 430 core\nvoid main() { broken syntax; }", false, "broken.vert"));
        CHECK(shader.getLastError().find("source='broken.vert'") != std::string::npos);
        CHECK(shader.getLastError().find("compiler log:") != std::string::npos);

        const auto vertex = fixture / "valid.vert";
        write(vertex, "#version 430 core\nout vec3 color;\nvoid main() { color = vec3(1); gl_Position = vec4(0); }");
        log.clear();
        CHECK(shader.createShader(robot2D::ShaderType::Vertex, vertex.string(), true));
        CHECK(shader.getLastError().empty());
        CHECK(log.text().find("Program linked; program=" + std::to_string(shader.getProgram()) +
            "; stage=vertex; source='" + vertex.string() + "'") != std::string::npos);
        CHECK(!shader.createShader(robot2D::ShaderType::Fragment,
            "#version 430 core\nin vec2 color; out vec4 pixel; void main() { pixel = vec4(color, 0, 1); }",
            false, "mismatched.frag"));
        CHECK(shader.getLastError().find("link failed") != std::string::npos);
        CHECK(shader.getLastError().find("mismatched.frag") != std::string::npos);
        log.clear();
        CHECK(shader.createShader(robot2D::ShaderType::Fragment,
            "#version 430 core\nin vec3 color; out vec4 pixel; void main() { pixel = vec4(color, 1); }",
            false, "valid.frag"));
        CHECK(shader.getLastError().empty());
        CHECK(log.text().find("Program linked; program=" + std::to_string(shader.getProgram()) +
            "; stage=fragment; source='valid.frag'") != std::string::npos);
        log.clear();
        shader.use();
        shader.unUse();
        CHECK(log.text().empty());
        CHECK(glGetError() == GL_NO_ERROR);
        glDeleteProgram(shader.getProgram());
    }
}

int main(int argc, char** argv) {
    if(!glfwInit()) return 77;
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    auto* window = glfwCreateWindow(32, 32, "OpenGL diagnostics tests", nullptr, nullptr);
    if(!window) {
        glfwTerminate(); return 77;
    }
    glfwMakeContextCurrent(window);
    int result = 0;
    if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) result = 77;
    else {
        try {
            CHECK(argc == 2);
            if(std::string(argv[1]) == "diagnostics") diagnostics();
            else if(std::string(argv[1]) == "shaders") shaders();
            else CHECK(false && "Unknown test case");
            std::cout << argv[1] << " passed\n";
        }
        catch(const std::exception& error) { std::cerr << error.what() << '\n'; result = 1; }
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return result;
}
