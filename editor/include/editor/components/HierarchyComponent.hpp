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

#include <list>
#include <optional>
#include <editor/Uuid.hpp>

namespace editor {

    class Scene;

    class HierarchyComponent {
    public:
        const std::optional<UUID>& getParentID() const { return m_parentID; }
        const std::list<UUID>& getChildrenIDs() const { return m_childrenIDs; }
    private:
        friend class Scene;

        std::optional<UUID> m_parentID;
        std::list<UUID> m_childrenIDs;
    };

}
