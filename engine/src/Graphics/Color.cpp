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

#include <robot2D/Graphics/Color.hpp>

namespace robot2D {
    constexpr float maxColorComponent = 255.F;
    
    Color::Color():
    red(0.F),
    green(0.F),
    blue(0.F),
    alpha(maxColorComponent){}


    Color::Color(const float& r, const float& g, const float& b, const float& alpha): red(r),
    green(g), blue(b), alpha(alpha) {}


    const Color Color::Black(0.F, 0.F, 0.F, maxColorComponent);
    const Color Color::White(maxColorComponent, maxColorComponent, maxColorComponent, maxColorComponent);
    const Color Color::Red(maxColorComponent, 0.F, 0.F, maxColorComponent);
    const Color Color::Green(0.F, maxColorComponent, 0.F, maxColorComponent);
    const Color Color::Blue(0.F, 0.F, maxColorComponent, maxColorComponent);
    const Color Color::Yellow(maxColorComponent, maxColorComponent, 0.F, maxColorComponent);
    const Color Color::Magenta(maxColorComponent, 0, maxColorComponent, maxColorComponent);
    const Color Color::Cyan(0.F, maxColorComponent, maxColorComponent, maxColorComponent);
    const Color Color::Transparent(0.F, 0.F, 0.F, 0.F);

    Color Color::toGL(const float& red, const float& green, const float& blue, const float& alpha) {
        return Color(red / maxColorComponent, green / maxColorComponent, blue / maxColorComponent, alpha / maxColorComponent);
    }

    Color Color::fromGL(const float& red, const float& green, const float& blue, const float& alpha) {
        return Color(red * maxColorComponent, green * maxColorComponent, blue * maxColorComponent,
                     alpha * maxColorComponent);
    }

    Color Color::toGL() const {
        return Color(red / maxColorComponent, green / maxColorComponent, blue / maxColorComponent, alpha / maxColorComponent);
    }
}