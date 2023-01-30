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
    ShaderHandler::ShaderHandler(): m_shaderProgram{-1} {}

    //todo rewrite reading file
    std::pair<bool, std::string> loadFromFile(const std::string& path) {
        std::fstream shaderFile(path);
        std::stringstream shaderStream;

        if (shaderFile.is_open())
            shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        std::string text = shaderStream.str();
        return {true, text};
    }

    int ShaderHandler::setupShader(ShaderType shaderType, const std::string& source, bool is_file) {
        const char* text{nullptr};
        if(is_file) {
            auto [result, shaderCode] = loadFromFile(source);
            if (!result) {
                RB_CORE_ERROR("ERROR::SHADER::LOAD_FAILED {0} \n", infoLog);
                return -1;
            }
            text = shaderCode.c_str();
        }
        else
            text = source.c_str();

        GLenum s_type;
        switch(shaderType) {
            case ShaderType::Vertex:
                s_type = GL_VERTEX_SHADER;
                break;
            case ShaderType::Fragment:
                s_type = GL_FRAGMENT_SHADER;
                break;
            case ShaderType::Geometry:
                s_type = GL_GEOMETRY_SHADER;
                break;
        }
        int shader = glCreateShader(s_type);

        glShaderSource(shader, 1, &text, nullptr);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            RB_CORE_ERROR("ERROR::SHADER::COMPILATION_FAILED {0}\n", infoLog);
            return -1;
        }

        return shader;
    }

    bool ShaderHandler::createShader(ShaderType shaderType, const std::string& source, bool is_path) {
        if(m_shaderProgram == -1)
            m_shaderProgram = glCreateProgram();
        int shader = setupShader(shaderType, source, is_path);
        if(shader == -1)
            return false;

        glAttachShader(m_shaderProgram, shader);
        glLinkProgram(m_shaderProgram);
        glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
            RB_CORE_ERROR("ERROR::SHADER::PROGRAM::LINKING_FAILED {0} \n", infoLog);
            return false;
        }
        glDeleteShader(shader);

        return true;
    }

    void ShaderHandler::use() const{
        glUseProgram(m_shaderProgram);
    }

    void ShaderHandler::unUse() const {
        glUseProgram(0);
    }

    int ShaderHandler::getProgram() const{
        return m_shaderProgram;
    }

    template<>
    void ShaderHandler::set<const float>(const std::string& param, const float value) const {
        glUniform1f(glGetUniformLocation(m_shaderProgram, param.c_str()), value);
    }
    
    template<>
    void ShaderHandler::set<int>(const std::string& param, const int value) const  {
        glUniform1i(glGetUniformLocation(m_shaderProgram, param.c_str()), value);
    }

    template<>
    void ShaderHandler::set<bool>(const std::string& param, const bool value) const  {
        glUniform1i(glGetUniformLocation(m_shaderProgram, param.c_str()), value);
    }

    template<>
    void ShaderHandler::set<float>(const std::string& param, const float value) const  {
        glUniform1f(glGetUniformLocation(m_shaderProgram, param.c_str()), value);
    }

    template<>
    void ShaderHandler::set<vec2f>(const std::string& param, const vec2f vec) const  {
        glUniform2f(glGetUniformLocation(m_shaderProgram, param.c_str()), vec.x, vec.y);
    }

    template<>
    void ShaderHandler::set<Color>(const std::string& param, const Color vec) const  {
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
        glDeleteProgram(m_shaderProgram);
    }

}