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
#include <vector>
#include <cstdlib>

#include <robot2D/Config.hpp>

namespace robot2D::ecs {
    using Bitset = uint32_t;

    ///
    class ROBOT2D_EXPORT_API Bitmask {
    public:
        Bitmask();
        Bitmask(const Bitset& bits);
        Bitmask(const Bitmask& bitmask) = default;
        Bitmask& operator=(const Bitmask& bitmask) = default;
        Bitmask(Bitmask&& bitmask) = default;
        Bitmask& operator=(Bitmask&& bitmask) = default;
        ~Bitmask() = default;

        Bitset getBitset() const;
        void setBitset(const Bitset& bitset);

        /// \brief
        bool matches(const Bitmask& other,
                     const Bitset& relevant = 0) const;

        /// \brief
        bool getBit(const unsigned int& pos) const;

        /// \brief
        void turnOnBit(const unsigned int& pos);

        /// \brief
        void turnOnBits(const Bitset& bitset);

        /// \brief
        void toggleBit(const unsigned int& pos);

        /// \brief
        void clear(const unsigned int& pos);

        /// \brief
        void Clear();
    private:
        Bitset m_bits;
    };

    // get mask from input components
    Bitmask configureMask(std::vector<Bitset> bits);
}