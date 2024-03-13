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
     * \brief RGBA Color format.
     * \details RGBA = Red, Green, Blue, Alpha.
     * Values in RGBA from 0 to 255, but in OpenGL(for example) from 0.0 to 1.0.
     */
    class ROBOT2D_EXPORT_API Color final {
    public:
        /// Defalut color is black (0, 0, 0, 255).
        Color();
        /// Custom color by RGBA components.
        Color(const float& red, const float& green, const float& blue,
              const float& alpha = 255.F);
        ~Color() = default;

        static const Color Black;       //!< Black predefined color
        static const Color White;       //!< White predefined color
        static const Color Red;         //!< Red predefined color
        static const Color Green;       //!< Green predefined color
        static const Color Blue;        //!< Blue predefined color
        static const Color Yellow;      //!< Yellow predefined color
        static const Color Magenta;     //!< Magenta predefined color
        static const Color Cyan;        //!< Cyan predefined color
        static const Color Transparent; //!< Transparent (black) predefined color

        /// Convert current object in OpenGL color format
        Color toGL() const;

        /// Convert from OpenGL 0 to 1 by RGBA 0 to 255 format.
        static Color fromGL(const float& red, const float& green, const float& blue, const float& alpha = 255.F);
        /// Convert from RGBA 0 to 255 format by OpenGL 0 to 1.
        static Color toGL(const float& red, const float& green, const float& blue, const float& alpha = 1.F);
    public:
        float red;
        float green;
        float blue;
        float alpha;
    };
}