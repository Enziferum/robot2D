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

namespace robot2D {
    /**
     * \brief Rather Window has own context you need Render information data.
     */
    struct ROBOT2D_EXPORT_API RenderContext {
        bool useBlending;
        bool useDepthTest;
        bool useLogs;

        RenderContext(const bool& blending = true, const bool& depth = false, const bool& logs = true);
        static const RenderContext Default;
    };
}