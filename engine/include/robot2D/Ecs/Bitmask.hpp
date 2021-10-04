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
#include <vector>

namespace robot2D::ecs {
    using Bitset = uint32_t;

    // todo hash code
    class Bitmask {
    public:
        Bitmask();
        Bitmask(const Bitset& bits);
        ~Bitmask() = default;

        Bitset getBitset() const;
        void setBitset(const Bitset& bitset);

        bool matches(const Bitmask& other,
                     const Bitset& relevant = 0);

        bool getBit(const unsigned int& pos) const;

        void turnOnBit(const unsigned int& pos);
        void turnOnBits(const Bitset& bitset);
        void toggleBit(const unsigned int& pos);
        void clear(const unsigned int& pos);

        void Clear();
    private:
        Bitset m_bits;
    };

    // get mask from input components
    Bitmask configureMask(std::vector<Bitset> bits);
}