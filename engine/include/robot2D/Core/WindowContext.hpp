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

#include <robot2D/Config.hpp>
#include "Vector2.hpp"

namespace robot2D {
    struct ROBOT2D_EXPORT_API WindowContext {
        enum class RenderApi {
            OpenGL3_3,
            OpenGL4_5
        };

        bool vsync;
        bool fullscreen;
        RenderApi renderApi;

        robot2D::vec2f minimumSize = {};
        robot2D::vec2f maximumSize = {};

        WindowContext(const bool& vsync = false, const bool& fullscreen = false,
                      const RenderApi& renderApi = WindowContext::RenderApi::OpenGL4_5);

        static const WindowContext Default;
    };

}
