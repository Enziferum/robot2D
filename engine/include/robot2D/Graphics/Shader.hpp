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

    enum class shaderType {
        vertex,
        fragment,
        geometry
    };


    class ROBOT2D_EXPORT_API ShaderHandler {
    public:
        ShaderHandler();
        ~ShaderHandler() = default;

        bool createShader(shaderType shader_type, const std::string& source, bool is_path);

        template<typename T>
        void set(const char* name, const T value) const;

        template<typename T>
        void setArray(const char* name, const T* value, const size_t& size) const;

        void use() const;
        void unUse() const;

        int getProgram() const;
    private:
        int setupShader(shaderType shader_type, const char* path, bool is_file = true);

    private:
        int shaderProgram;
        int success;
        char infoLog[512];
    };

}
