/*********************************************************************
(c) Alex Raag 2024
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

namespace editor {

    class class_id {
    public:
        class_id(const char* name);

        // TODO(a.raag): can't be default due to atomic inside
        class_id(const class_id& other) = default;
        class_id& operator=(const class_id& other) = default;
        class_id(class_id&& other) = default;
        class_id& operator=(class_id&& other) = default;

        ~class_id() = default;

        int getIndex() const { return m_index; }
        const char* const getName() const { return m_name; }


        bool operator == (const class_id& another) const noexcept
        {
            return m_index == another.m_index;
        }
    private:
        const char* const m_name;
        int m_index;
    };

#define DECLARE_COMPONENT_ID() \
    static const class_id& id() noexcept;

} // namespace editor