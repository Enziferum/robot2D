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

#include <robot2D/Ecs/EntityManager.hpp>
#include <robot2D/Ecs/Entity.hpp>

namespace robot2D::ecs {

    Entity::Entity(EntityID entityId): m_entityManager(nullptr), m_id(entityId){}
    Entity::Entity(EntityManager* entityManager, const EntityID& id):
    m_entityManager(entityManager),
    m_id(id){}

    Entity::Entity(const Entity& other):
        m_entityManager{other.m_entityManager},
        m_id{other.m_id},
        m_needAddToScene{other.m_needAddToScene}{}

    Entity::Entity(Entity&& other):
        m_entityManager{std::move(other.m_entityManager)},
        m_id{std::move(other.m_id)},
        m_needAddToScene{other.m_needAddToScene}
    {}

    Entity& Entity::operator=(const Entity& other) {
        if(this == &other)
            return *this;
        m_entityManager = other.m_entityManager;
        m_id = other.m_id;
        m_needAddToScene = other.m_needAddToScene;
        return *this;
    }

    bool operator==(const Entity& left, const Entity& right) {
        return (left.m_id == right.m_id);
    }

    bool operator != (const Entity& l, const Entity& r) {
        return !(l == r);
    }

    bool operator < (const Entity& l, const Entity& r) {
        return (l.m_id < r.m_id);
    }

    Bitmask Entity::getComponentMask() const {
        return m_entityManager -> getComponentBitmask(*this);
    }

    bool Entity::destroyed() const {
        return !m_entityManager || m_entityManager -> entityDestroyed(*this);
    }

    void Entity::removeSelf() {
        m_entityManager -> removeEntityFromScene(*this);
    }

}
