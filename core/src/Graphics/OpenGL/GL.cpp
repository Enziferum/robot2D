#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Util/Logger.hpp>
#include <algorithm>
#include <mutex>
#include <unordered_map>

namespace robot2D {
    namespace {
        std::mutex diagnosticsMutex;
        GLDiagnosticsOptions diagnosticsOptions;
        std::unordered_map<std::string, unsigned int> messageCounts;
        thread_local const char* currentFile = "unavailable (unwrapped or asynchronous call)";
        thread_local const char* currentOperation = "unavailable";
        thread_local std::uint_fast32_t currentLine = 0;

        void report(spdlog::level::level_enum level, const std::string& message) {
            bool last = false;
            {
                std::lock_guard<std::mutex> lock(diagnosticsMutex);
                if(messageCounts.size() >= 256 && messageCounts.find(message) == messageCounts.end())
                    messageCounts.clear();
                auto& count = messageCounts[message];
                if(diagnosticsOptions.repeatLimit != 0 && count >= diagnosticsOptions.repeatLimit)
                    return;
                ++count;
                last = diagnosticsOptions.repeatLimit != 0 && count == diagnosticsOptions.repeatLimit;
            }
            const auto log = logger::Log::getCoreLogger() ? logger::Log::getCoreLogger() : spdlog::default_logger();
            log -> log(level, "{}{}", message, last ? " [further identical messages suppressed]" : "");
        }

#if defined(ROBOT2D_WINDOWS) || defined(ROBOT2D_LINUX)
        const char* sourceName(GLenum source) {
            switch(source) {
            case GL_DEBUG_SOURCE_API: return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "window-system";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "shader-compiler";
            case GL_DEBUG_SOURCE_THIRD_PARTY: return "third-party";
            case GL_DEBUG_SOURCE_APPLICATION: return "application";
            default: return "other";
            }
        }

        const char* typeName(GLenum type) {
            switch(type) {
            case GL_DEBUG_TYPE_ERROR: return "error";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "deprecated-behavior";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "undefined-behavior";
            case GL_DEBUG_TYPE_PORTABILITY: return "portability";
            case GL_DEBUG_TYPE_PERFORMANCE: return "performance";
            case GL_DEBUG_TYPE_MARKER: return "marker";
            case GL_DEBUG_TYPE_PUSH_GROUP: return "push-group";
            case GL_DEBUG_TYPE_POP_GROUP: return "pop-group";
            default: return "other";
            }
        }

        void APIENTRY debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity,
            GLsizei length, const GLchar* message, const void*) noexcept {
            // No OpenGL calls and no exceptions crossing the driver callback boundary.
            try {
                {
                    std::lock_guard<std::mutex> lock(diagnosticsMutex);
                    if(!diagnosticsOptions.notifications && severity == GL_DEBUG_SEVERITY_NOTIFICATION)
                        return;
                }
                auto level = spdlog::level::warn;
                const char* severityName = "unknown";
                if(severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
                    level = spdlog::level::debug;
                    severityName = "notification";
                }
                if(severity == GL_DEBUG_SEVERITY_HIGH) {
                    level = spdlog::level::err;
                    severityName = "high";
                }
                else if(severity == GL_DEBUG_SEVERITY_MEDIUM) {
                    level = type == GL_DEBUG_TYPE_ERROR ? spdlog::level::err : spdlog::level::warn;
                    severityName = "medium";
                }
                else if(severity == GL_DEBUG_SEVERITY_LOW) {
                    level = spdlog::level::warn;
                    severityName = "low";
                }
                report(level, fmt::format("[OpenGL] source={}; type={}; severity={}; id={}; "
                    "operation={}; location={}:{}; driver: {}",
                    sourceName(source), typeName(type), severityName, id,
                    currentOperation, currentFile, currentLine,
                    message ? std::string(message, length > 0 ? static_cast<std::size_t>(length)
                                                             : std::char_traits<char>::length(message)) : ""));
            }
            catch(...) {}
        }
#endif

        const char* errorDescription(GLenum error) {
            switch(error) {
            case GL_INVALID_ENUM: return "GL_INVALID_ENUM: an enum argument is not accepted by this operation";
            case GL_INVALID_VALUE: return "GL_INVALID_VALUE: a numeric argument is outside the accepted range";
            case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION: operation is incompatible with current GL state";
            case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION: framebuffer is incomplete";
            case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY: driver could not allocate required memory";
            default: return "unknown OpenGL error";
            }
        }
    }

    priv::GLCallScope::GLCallScope(const char* file, std::uint_fast32_t line, const char* operation) :
        m_previousFile(currentFile), m_previousOperation(currentOperation), m_previousLine(currentLine) {
        currentFile = file;
        currentOperation = operation;
        currentLine = line;
    }

    priv::GLCallScope::~GLCallScope() {
        currentFile = m_previousFile;
        currentOperation = m_previousOperation;
        currentLine = m_previousLine;
    }

    void configureGLDiagnostics(const GLDiagnosticsOptions& options) {
        {
            std::lock_guard<std::mutex> lock(diagnosticsMutex);
            diagnosticsOptions = options;
            messageCounts.clear();
        }
#if defined(ROBOT2D_WINDOWS) || defined(ROBOT2D_LINUX)
        if(!glDebugMessageCallback || !glDebugMessageControl) {
            report(spdlog::level::warn, "[OpenGL] Debug callback unavailable; wrapped calls still report glGetError");
            return;
        }
        if(!options.debugOutput) {
            glDebugMessageCallback(nullptr, nullptr);
            glDisable(GL_DEBUG_OUTPUT);
            return;
        }
        glEnable(GL_DEBUG_OUTPUT);
        if(options.synchronous)
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        else
            glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debugMessage, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,
            0, nullptr, options.notifications ? GL_TRUE : GL_FALSE);
#endif
    }

    void logGLContextInfo() {
        const auto value = [](GLenum key) {
            const auto* text = glGetString(key);
            return text ? reinterpret_cast<const char*>(text) : "unavailable";
        };
        GLint textureUnits = 0;
        GLint attributes = 0;
        GLint colorAttachments = 0;
        GLint flags = 0;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureUnits);
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attributes);
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &colorAttachments);
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        bool debugContext = false;
#if defined(GL_CONTEXT_FLAG_DEBUG_BIT)
        debugContext = (flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0;
#endif
        report(spdlog::level::info, fmt::format(
                "[OpenGL] vendor='{}'; renderer='{}'; version='{}'; GLSL='{}'; debug-context={}; "
                "limits: fragment-texture-units={}, vertex-attributes={}, color-attachments={}",
                value(GL_VENDOR), value(GL_RENDERER), value(GL_VERSION), value(GL_SHADING_LANGUAGE_VERSION),
                debugContext,
                textureUnits, attributes, colorAttachments));
    }

    void labelGLObject(unsigned int type, unsigned int id, const std::string& name) {
#if defined(ROBOT2D_WINDOWS) || defined(ROBOT2D_LINUX)
        if(id != 0 && glObjectLabel) {
            GLint maxLength = 0;
            glGetIntegerv(GL_MAX_LABEL_LENGTH, &maxLength);
            if(maxLength > 1)
                glObjectLabel(type, id, static_cast<GLsizei>(std::min(name.size(),
                    static_cast<std::size_t>(maxLength - 1))), name.c_str());
        }
#endif
    }

    bool check_gl_errors(const char* file, std::uint_fast32_t line, const char* operation) {
        bool success = true;
        for(auto error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
            success = false;
            report(spdlog::level::err, fmt::format("[OpenGL] operation='{}'; {} (0x{:X}); location={}:{}",
                operation, errorDescription(error), error, file, line));
        }
        return success;
    }
}
