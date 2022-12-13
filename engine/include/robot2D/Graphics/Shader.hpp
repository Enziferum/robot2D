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

#pragma once

#include <string>
#include <robot2D/Core/Vector2.hpp>

namespace robot2D {
    /// Specify Shader Type to Render Pipeline
    enum class shaderType {
        vertex,
        fragment,
        geometry
    };

    /// \brief Specify GPU Pipeline process.
    /// \details Modern Graphics API (OpenGL 3+ / Vulkan / DirectX) obey to explain how to work \n
    /// with input information during Render Pipeline.
    class ROBOT2D_EXPORT_API ShaderHandler {
    public:
        /// Only initialize class. To create use specific method.
        ShaderHandler();
        ~ShaderHandler() = default;

        /// \brief Create specific shader.
        /// \details
        bool createShader(shaderType shader_type, const std::string& source, bool is_path);

        /// \brief Set to shader program variable by name and data
        template<typename T>
        void set(const char* name, const T value) const;

        void setMatrix(const char* name, const float* value) const;

        /// \brief Set to shader program variable by name and data as array
        template<typename T>
        void setArray(const char* name, const T* value, const size_t& size) const;

        /// You must directly to enable
        void use() const;

        /// You must directly to disable
        void unUse() const;

        /// Raw shader program in Graphics API specific way
        int getProgram() const;
    private:

        /// Preparing process
        int setupShader(shaderType shader_type, const char* path, bool is_file = true);

    private:
        int shaderProgram;
        int success;
        char infoLog[512];
    };

}
