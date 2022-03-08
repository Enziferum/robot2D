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

#include <unordered_map>
#include <functional>
#include <cstdint>
#include <string>

namespace editor {


    class Uuid {
    public:
        Uuid();
        Uuid(uint64_t uuid);
        Uuid(const Uuid&) = default;

        operator uint64_t() const { return m_uuid; }
    private:
        uint64_t m_uuid;
    };

    class UuidManager {
    public:
        UuidManager();
        ~UuidManager();


    private:
        std::unordered_map<uint64_t, std::string> m_uuids;
    };

}
namespace std {
    template<>
    struct hash<editor::Uuid> {
        std::size_t operator()(const editor::Uuid& uuid) const {
            return hash<uint64_t>()((uint64_t)uuid);
        }
    };
}