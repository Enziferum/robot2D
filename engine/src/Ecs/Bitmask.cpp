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

#include <algorithm>
#include <robot2D/Ecs/Bitmask.hpp>

namespace robot2D::ecs {
    Bitmask::Bitmask() : m_bits(0) {

    }

    Bitmask::Bitmask(const Bitset& bits): m_bits(bits) {

    }

    Bitset Bitmask::getBitset() const {
        return m_bits;
    }

    void Bitmask::setBitset(const ecs::Bitset& bitset) {
        m_bits = bitset;
    }

    bool Bitmask::getBit(const unsigned int& pos) const {
        return ((m_bits&(1 << pos)) != 0);
    }

    void Bitmask::turnOnBit(const unsigned int& pos) {
        m_bits |= (1 << pos);
    }

    void Bitmask::turnOnBits(const Bitset& bitset) {
        m_bits |= bitset;
    }

    void Bitmask::toggleBit(const unsigned int& pos) {
        m_bits ^= (1 << pos);
    }

    bool Bitmask::matches(const Bitmask &other, const Bitset& relevant) {
        return (relevant
                        ? ((other.getBitset() & relevant) == (m_bits & relevant))
                        : (m_bits == other.m_bits));
    }

    void Bitmask::clear(const unsigned int& pos) {
        m_bits &= ~(1 << pos);
    }

    void Bitmask::Clear() {
        m_bits = 0;
    }

    Bitmask configureMask(std::vector<Bitset> bits) {
        // sort because todo
        std::sort(bits.begin(), bits.end(), [](const Bitset& left, const Bitset& right) -> bool {
            return left < right;
        });
        Bitmask mask;
        for(auto it: bits)
            mask.turnOnBit(it);
        return mask;
    }
}