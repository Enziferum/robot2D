/*********************************************************************
   (c) Alex Raag 2021
   https://github.com/Enziferum
   robot2D - Zlib license.
   This software is provided 'as-is', without any express or
   implied warranty. In no event will the authors be held
   liable for any damages arising from the use of this software.
   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute
   it freely, subject to the following restrictions:
   1. The origin of this software must not be misrepresented;
   you must not claim that you wrote the original software.
   If you use this software in a product, an acknowledgment
   in the product documentation would be appreciated but
   is not required.
   2. Altered source versions must be plainly marked as such,
   and must not be misrepresented as being the original software.
   3. This notice may not be removed or altered from any
   source distribution.
*********************************************************************/

#include <sstream>
#include <fstream>
#include <tuple>

#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Graphics/Transform.hpp>

#include <robot2D/Util/Logger.hpp>

namespace robot2D {
    ShaderHandler::ShaderHandler() : m_shaderProgram{-1} {
    }

    namespace {
        const char* stageName(ShaderType type) {
            switch(type) {
            case ShaderType::Vertex: return "vertex";
            case ShaderType::Fragment: return "fragment";
            case ShaderType::Geometry: return "geometry";
            }
            return "unknown";
        }

        std::string shaderLog(unsigned int id, bool program) {
            GLint size = 0;
            if(program)
                glGetProgramiv(id, GL_INFO_LOG_LENGTH, &size);
            else
                glGetShaderiv(id, GL_INFO_LOG_LENGTH, &size);
            if(size <= 1)
                return {};
            std::string result(static_cast<std::size_t>(size), '\0');
            GLsizei written = 0;
            if(program)
                glGetProgramInfoLog(id, size, &written, result.data());
            else
                glGetShaderInfoLog(id, size, &written, result.data());
            result.resize(static_cast<std::size_t>(written));
            return result;
        }
    }

    int ShaderHandler::setupShader(ShaderType shaderType, const std::string& source, bool is_file,
        const std::string& sourceName) {
        const auto fail = [&](const std::string& reason) {
            m_lastError = fmt::format("[GLSL] stage={}; source='{}'; {}", stageName(shaderType), sourceName, reason);
            RB_CORE_ERROR("{}", m_lastError);
            return -1;
        };
        std::string code = source;
        if(is_file) {
            std::ifstream input(source, std::ios::binary);
            if(!input)
                return fail("read failed: could not open shader file");
            std::ostringstream contents;
            contents << input.rdbuf();
            if(input.bad())
                return fail("read failed: I/O error while reading shader file");
            code = contents.str();
        }
        if(code.empty())
            return fail("compile failed: shader source is empty");

        GLenum type;
        switch(shaderType) {
        case ShaderType::Vertex: type = GL_VERTEX_SHADER; break;
        case ShaderType::Fragment: type = GL_FRAGMENT_SHADER; break;
        case ShaderType::Geometry: type = GL_GEOMETRY_SHADER; break;
        default: return fail("compile failed: unsupported shader stage");
        }
        const GLuint shader = glCreateShader(type);
        if(shader == 0)
            return fail("glCreateShader returned 0");
#if !defined(ROBOT2D_MACOS)
        labelGLObject(GL_SHADER, shader, sourceName);
#endif
        const GLchar* text = code.c_str();
        const GLint length = static_cast<GLint>(code.size());
        glShaderSource(shader, 1, &text, &length);
        glCall(glCompileShader, shader);
        GLint success = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        const auto log = shaderLog(shader, false);
        if(success != GL_TRUE) {
            glDeleteShader(shader);
            return fail("compile failed; compiler log:\n" + log);
        }
        if(!log.empty())
            RB_CORE_WARN("[GLSL] stage={}; source='{}'; compiler log:\n{}", stageName(shaderType), sourceName, log);
        return static_cast<int>(shader);
    }

    bool ShaderHandler::createShader(ShaderType shaderType, const std::string& source, bool is_path,
        const std::string& sourceName) {
        m_lastError.clear();
        const auto name = is_path ? source : sourceName.empty() ? "<unnamed inline GLSL>" : sourceName;
        const int shader = setupShader(shaderType, source, is_path, name);
        if(shader == -1)
            return false;
        if(m_shaderProgram == -1)
            m_shaderProgram = static_cast<int>(glCreateProgram());
        if(m_shaderProgram == 0) {
            glDeleteShader(shader);
            m_shaderProgram = -1;
            m_lastError = fmt::format("[GLSL] source='{}'; glCreateProgram returned 0", name);
            RB_CORE_ERROR("{}", m_lastError);
            return false;
        }
#if !defined(ROBOT2D_MACOS)
        labelGLObject(GL_PROGRAM, m_shaderProgram, name);
#endif
        glAttachShader(m_shaderProgram, shader);
        glCall(glLinkProgram, m_shaderProgram);
        GLint success = GL_FALSE;
        glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
        const auto log = shaderLog(m_shaderProgram, true);
        // Successful stages stay attached for the existing incremental creation API.
        // Deleting marks them for release when detached or when the program is deleted.
        glDeleteShader(shader);
        if(success != GL_TRUE) {
            glDetachShader(m_shaderProgram, shader);
            m_lastError = fmt::format("[GLSL] program={}; stage={}; source='{}'; link failed; linker log:\n{}",
                    m_shaderProgram, stageName(shaderType), name, log);
            RB_CORE_ERROR("{}", m_lastError);
            return false;
        }
        if(!log.empty())
            RB_CORE_WARN("[GLSL] program={}; source='{}'; linker log:\n{}", m_shaderProgram, name, log);
        RB_CORE_INFO("[GLSL] Program linked; program={}; stage={}; source='{}'",
            m_shaderProgram, stageName(shaderType), name);
        return true;
    }

    void ShaderHandler::use() const {
        glCall(glUseProgram, m_shaderProgram);
    }

    void ShaderHandler::unUse() const {
        glCall(glUseProgram, 0);
    }

    int ShaderHandler::getProgram() const {
        return m_shaderProgram;
    }

    template<>
    void ShaderHandler::set<const float>(const std::string& param, const float value) const {
        glUniform1f(glGetUniformLocation(m_shaderProgram, param.c_str()), value);
    }

    template<>
    void ShaderHandler::set<int>(const std::string& param, const int value) const {
        glUniform1i(glGetUniformLocation(m_shaderProgram, param.c_str()), value);
    }

    template<>
    void ShaderHandler::set<bool>(const std::string& param, const bool value) const {
        glUniform1i(glGetUniformLocation(m_shaderProgram, param.c_str()), value);
    }

    template<>
    void ShaderHandler::set<float>(const std::string& param, const float value) const {
        glUniform1f(glGetUniformLocation(m_shaderProgram, param.c_str()), value);
    }

    template<>
    void ShaderHandler::set<vec2f>(const std::string& param, const vec2f vec) const {
        glUniform2f(glGetUniformLocation(m_shaderProgram, param.c_str()), vec.x, vec.y);
    }

    template<>
    void ShaderHandler::set<vec2i>(const std::string& param, const vec2i vec) const {
        glUniform2i(glGetUniformLocation(m_shaderProgram, param.c_str()), vec.x, vec.y);
    }

    template<>
    void ShaderHandler::set<Color>(const std::string& param, const Color vec) const {
        glUniform4f(glGetUniformLocation(m_shaderProgram, param.c_str()), vec.red, vec.green,
            vec.blue, vec.alpha);
    }


    template<>
    void ShaderHandler::setArray(const std::string& param, const int* value, const size_t& size) const {
        glUniform1iv(glGetUniformLocation(m_shaderProgram, param.c_str()),
            static_cast<GLsizei>(size), value);
    }


    void ShaderHandler::setMatrix(const std::string& param, const float* value) const {
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, param.c_str()),
            1, GL_FALSE, value);
    }

    ShaderHandler::~ShaderHandler() {
        //        if(m_shaderProgram != -1)
        //            glDeleteShader(m_shaderProgram);
    }

}
