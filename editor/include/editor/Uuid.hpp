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

#include <unordered_map>
#include <functional>
#include <cstdint>
#include <string>

namespace editor {

    class UUID {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID&) = default;

        operator uint64_t() const { return m_uuid; }
        bool operator==(const UUID& other) {
            return m_uuid == other.m_uuid;
        }
    private:
        uint64_t m_uuid;
    };
}

namespace std {
    template<>
    struct hash<editor::UUID> {
        std::size_t operator()(const editor::UUID& uuid) const {
            return hash<uint64_t>()((uint64_t)uuid);
        }
    };
}