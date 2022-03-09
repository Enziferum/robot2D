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
#include <robot2D/Config.hpp>

namespace robot2D {
    class ROBOT2D_EXPORT_API Time {
    public:
        Time(int64_t time = 0LL);
        ~Time() = default;

        float asSeconds() const { return m_time / 1000000.F; }
        float asMilliSeconds() const { return m_time / 1000.F; }
        int64_t asMicroSeconds() const { return m_time; }
    private:
        int64_t m_time;
    };
}