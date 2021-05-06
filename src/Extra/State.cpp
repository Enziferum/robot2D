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

#include <robot2D/Extra/State.hpp>

namespace robot2D{
    State::State(IStateMachine& machine):
        m_machine(machine),
        m_window(m_machine.getWindow()),
        m_active(false){}

    State::~State() {}

    bool State::isActive() const {
        return m_active;
    }

    void State::setActive(const bool& flag) {
        m_active = flag;
    }
}
