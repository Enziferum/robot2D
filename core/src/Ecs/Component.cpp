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

#include <algorithm>
#include <robot2D/Ecs/Component.hpp>

namespace robot2D::ecs {

    ComponentManager::ComponentManager(): m_indices() {}

    ComponentManager::ID ComponentManager::getIDFromIndex(const UniqueType& uniqueType) {
        auto found = std::find_if(m_indices.begin(), m_indices.end(), [&](const UniqueType& type) {
            return uniqueType == type;
        });

        if(found == m_indices.end()) {
            m_indices.emplace_back(uniqueType);
            return m_indices.size() - 1;
        }

        return std::distance(m_indices.begin(), found);
    }

    bool ComponentManager::cloneSelf(ComponentManager& cloneManager) {
        cloneManager.m_indices = m_indices;
        return true;
    }

    bool ComponentManager::clearSelf() {
        m_indices.clear();
        return true;
    }

}