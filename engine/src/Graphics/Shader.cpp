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
#include <robot2D/Util/Logger.hpp>

namespace robot2D{
    ShaderHandler::ShaderHandler() {
        shaderProgram = glCreateProgram();
    }

    //todo rewrite reading file
    std::tuple<bool, std::string> loadFromFile(const char* path) {
        std::fstream shaderFile(path);
        std::stringstream shaderStream;

        if (shaderFile.is_open())
            shaderStream << shaderFile.rdbuf();
        std::string text = shaderStream.str();
        return std::make_tuple(true, text);
    }

    int ShaderHandler::setupShader(shaderType shader_type, const char* path, bool is_file) {
        const char* text;
        if(is_file) {
            bool ok;
            std::string shaderCode;
            std::tie(ok, shaderCode) = loadFromFile(path);
            if (!ok) {
                LOG_ERROR("ERROR::SHADER::LOAD_FAILED % \n", infoLog)
                return -1;
            }
            text = shaderCode.c_str();
        }
        else
            text = path;
        GLenum s_type;
        if(shader_type == shaderType::vertex)
            s_type = GL_VERTEX_SHADER;
        if(shader_type == shaderType::fragment)
            s_type = GL_FRAGMENT_SHADER;

        int shader = glCreateShader(s_type);

        glShaderSource(shader, 1, &text, NULL);
        glCompileShader(shader);
        // check for shader compile errors
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            LOG_ERROR("ERROR::SHADER::COMPILATION_FAILED %\n", infoLog)
        }

        return shader;
    }

    bool ShaderHandler::createShader(shaderType shader_type, const std::string& source, bool is_path) {
        int shader = setupShader(shader_type, source, is_path);
        glAttachShader(shaderProgram, shader);
        glLinkProgram(shaderProgram);
        // check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            LOG_ERROR("ERROR::SHADER::PROGRAM::LINKING_FAILED % \n", infoLog)
        }
        glDeleteShader(shader);

        return true;
    }


    void ShaderHandler::use() const{
        glUseProgram(shaderProgram);
    }

    void ShaderHandler::set_parameter(const char* name, const int& value) const{
        glUniform1i(glGetUniformLocation(shaderProgram, name), value);
        glUniform1f(glGetUniformLocation(shaderProgram, name), value);
    }

    void ShaderHandler::set_parameter(const char* name, const float& value) const{
        glUniform1f(glGetUniformLocation(shaderProgram, name), value);
    }

    void ShaderHandler::set_parameter(const char* name, const float *matrix) const{
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name),
                           1, GL_FALSE, matrix);
    }

    void ShaderHandler::set_parameter(const char *name, const float& x, const float& y, const float& z) const{
        glUniform3f(glGetUniformLocation(shaderProgram, name), x, y, z);
    }

    void ShaderHandler::set_parameter(const char *name, const vec2f &vec) const{
        glUniform2f(glGetUniformLocation(shaderProgram, name), vec.x, vec.y);
    }

    void
    ShaderHandler::set_parameter(const char *name, const float &x, const float &y, const float &z, const float &w) const{
        glUniform4f(glGetUniformLocation(shaderProgram, name), x, y, z, w);
    }

    int ShaderHandler::getProgram() const{
        return shaderProgram;
    }
}