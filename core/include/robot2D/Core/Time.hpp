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

    /** @brief A signed duration stored as an exact count of microseconds.
     * This is an interval, not a calendar timestamp. Negative values are allowed.
     * Floating-point conversions are approximate; use asMicroSeconds() for the
     * exact stored value. Copies are independent values.
     */
    class ROBOT2D_EXPORT_API Time {
    public:
        /// Constructs from microseconds; the default value is zero.
        Time(std::int64_t microseconds = 0LL);
        Time(const Time& other) = default;
        Time& operator=(const Time& other) = default;
        Time(Time&& other) = default;
        Time& operator=(Time&& other) = default;
        ~Time() = default;

        /// Converts to seconds, rounding to float after division in double.
        float asSeconds() const {
            return static_cast<float>(static_cast<double>(m_time) / 1000000.0);
        }

        /// Converts to milliseconds, rounding to float after division in double.
        float asMilliSeconds() const {
            return static_cast<float>(static_cast<double>(m_time) / 1000.0);
        }

        /// Returns the exact signed count of microseconds, without conversion.
        std::int64_t asMicroSeconds() const {
            return m_time;
        }
    private:
        std::int64_t m_time;
    };
}
