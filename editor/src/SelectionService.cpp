/*********************************************************************
(c) Alex Raag 2026
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
#include <editor/SelectionService.hpp>

namespace editor {

    void SelectionService::selectEntity(UUID uuid) {
        m_selectedEntities.clear();
        addEntity(uuid);
    }

    void SelectionService::selectEntities(const std::vector<UUID>& entities,
                                          std::optional<UUID> active) {
        auto selectedEntities = entities;
        clear();
        for(auto uuid: selectedEntities)
            addEntity(uuid);
        if(active && hasEntity(*active))
            m_activeEntity = active;
    }

    void SelectionService::addEntity(UUID uuid) {
        if(!hasEntity(uuid))
            m_selectedEntities.emplace_back(uuid);
        m_activeEntity = uuid;
    }

    void SelectionService::removeEntity(UUID uuid) {
        m_selectedEntities.erase(std::remove_if(m_selectedEntities.begin(), m_selectedEntities.end(),
                                               [uuid](UUID item) { return item == uuid; }),
                                 m_selectedEntities.end());
        if(m_activeEntity && *m_activeEntity == uuid) {
            if(m_selectedEntities.empty())
                m_activeEntity.reset();
            else
                m_activeEntity = m_selectedEntities.back();
        }
    }

    void SelectionService::toggleEntity(UUID uuid) {
        if(hasEntity(uuid))
            removeEntity(uuid);
        else
            addEntity(uuid);
    }

    void SelectionService::clear() {
        m_selectedEntities.clear();
        m_activeEntity.reset();
    }

    bool SelectionService::hasEntity(UUID uuid) const {
        return std::any_of(m_selectedEntities.begin(), m_selectedEntities.end(),
                           [uuid](UUID item) { return item == uuid; });
    }

    bool SelectionService::empty() const {
        return m_selectedEntities.empty();
    }

    const std::vector<UUID>& SelectionService::getSelectedEntities() const {
        return m_selectedEntities;
    }

    std::optional<UUID> SelectionService::lastActive() const {
        return m_activeEntity;
    }

}
