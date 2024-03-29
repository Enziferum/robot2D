/*********************************************************************
(c) Alex Raag 2023
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

    /**
     * \brief Time wraps raw variable format and allow to use in Human.
     * \details Formats:
     * Seconds, Milli / Micro seconds.
     * Also returning format for Clock entity.
     */
    class ROBOT2D_EXPORT_API Time {
    public:
        Time(int64_t time = 0LL);
        Time(const Time& other) = default;
        Time& operator=(const Time& other) = default;
        Time(Time&& other) = default;
        Time& operator=(Time&& other) = default;
        ~Time() = default;

        float asSeconds() const { return static_cast<float>(m_time) / 1000000.F; }

        float asMilliSeconds() const { return static_cast<float>(m_time) / 1000.F; }

        int64_t asMicroSeconds() const { return m_time; }
    private:
        int64_t m_time;
    };
}