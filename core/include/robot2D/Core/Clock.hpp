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
#include <cstddef>
#include <robot2D/Config.hpp>
#include "Time.hpp"

namespace robot2D {
    /** @brief Measures elapsed intervals with std::chrono::steady_clock.
     * Wall-clock adjustments cannot make intervals negative. Results are whole
     * microseconds, truncated toward zero; the native clock resolution may be
     * coarser. This is elapsed time, not CPU time or a calendar timestamp.
     * Copies preserve both reference points and the restart count, then evolve
     * independently. Concurrent access involving restart/assignment requires
     * external synchronization. Intervals must fit the native clock duration
     * and the signed microsecond representation of Time.
     */
    class ROBOT2D_EXPORT_API Clock {
    public:
        /// Starts both the lifetime and restart interval at the same instant.
        Clock();
        Clock(const Clock& other) = default;
        Clock& operator=(const Clock& other) = default;
        Clock(Clock&& other) = default;
        Clock& operator=(Clock&& other) = default;
        ~Clock() = default;

        /** @brief Returns elapsed time since the preceding restart, then resets it.
         * The first call measures from construction. Increments getRestartsCount()
         * once and leaves the lifetime measured by duration() unchanged.
         * Sub-microsecond remainder is carried to the next restart instead of
         * discarded, so rapid calls preserve elapsed time; a reported interval
         * can include less than one microsecond from the preceding interval.
         */
        Time restart();

        /// Returns elapsed time since construction, without changing either baseline.
        Time duration() const;

        /// Number of restart calls; unsigned size_t wraps modulo its value range.
        std::size_t getRestartsCount() const {
            return m_restartCount;
        }
    private:
        std::chrono::steady_clock::time_point m_start;
        std::chrono::steady_clock::time_point m_restart;
        std::size_t m_restartCount;
    };
}
