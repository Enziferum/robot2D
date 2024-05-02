/*********************************************************************
(c) Alex Raag 2024
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
#include <editor/EventBinder.hpp>

namespace robot2D {

    template<>
    const Event::MouseButtonEvent& eventAs(const robot2D::Event& event) {
        return event.mouse;
    }

    template<>
    const Event::MouseWheelEvent& eventAs(const robot2D::Event& event) {
        return event.wheel;
    }

    template<>
    const Event::MouseMoveEvent& eventAs(const robot2D::Event& event) {
        return event.move;
    }

    template<>
    const Event::KeyboardEvent& eventAs(const robot2D::Event& event) {
        return event.key;
    }

    template<>
    const Event::SizeEvent& eventAs(const robot2D::Event& event) {
        return event.size;
    }
}

namespace editor  {

    EventBinder::IFunc::~IFunc() {}
} // namespace editor