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
#include <chrono>
#include <robot2D/Config.hpp>
#include "Time.hpp"

namespace robot2D {
/**
 * \brief OS - independent Time clocking object.
 * \details
 */
    class ROBOT2D_EXPORT_API Clock {
    public:
        Clock();
        Clock(const Clock& other) = default;
        Clock& operator=(const Clock& other) = default;
        Clock(Clock&& other) = default;
        Clock& operator=(Clock&& other) = default;
        ~Clock() = default;

        /// Returns process delta in Time format.
        Time restart();
        /// Duration time from creation of clock.
        Time duration() const;

        std::size_t getRestartsCount() const { return m_restartCount; }
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_restart;
        std::size_t m_restartCount;
    };
}