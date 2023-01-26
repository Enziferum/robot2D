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

#include <cstdint>
#include <utility>

#include <robot2D/Config.hpp>
#if defined(ROBOT2D_WINDOWS)
    #include <robot2D/internal/glad.h>

#elif defined(ROBOT2D_MACOS)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#elif defined(ROBOT2D_LINUX)
    #include <ext/glad.h>
#endif

namespace robot2D {
    #define glCall(function, ...) glCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)

    bool check_gl_errors(const char* file, std::uint_fast32_t line);

    template<typename glFunction, typename ... Args>
    auto glCallImpl(const char* file,
                    std::uint_fast32_t line,
                    glFunction function,
                    Args&& ...args) ->
    typename std::enable_if_t<
            !std::is_same_v<void, decltype(function(args...))>,
            decltype(function(args...))>
    {
        auto ret = function(std::forward<Args>(args)...);
        check_gl_errors(file, line);
        return ret;
    }

    template<typename glFunction, typename ... Args>
    auto glCallImpl(const char* file,
                    std::uint_fast32_t line,
                    glFunction function,
                    Args&& ...args) ->
    typename std::enable_if_t<
            std::is_same_v<void, decltype(function(args...))>, bool>
    {
        function(std::forward<Args>(args)...);
        return check_gl_errors(file, line);
    }
}