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

#include <robot2D/Ecs/EntityManager.hpp>
#include <robot2D/Ecs/Entity.hpp>

namespace robot2D::ecs {

    Entity::Entity(): m_entityManager(nullptr), m_id(0), m_tag("") {}
    Entity::Entity(EntityManager* entityManager, const EntityID& id):
    m_entityManager(entityManager),
    m_id(id) {}

    bool operator==(const Entity& left, const Entity& right) {
        return (left.m_id == right.m_id) && (left.m_tag == right.m_tag);
    }

    Bitmask Entity::getComponentMask() const {
        return m_entityManager -> getComponentBitmask(*this);
    }

    Entity::Entity(const Entity& other): m_entityManager{other.m_entityManager},
    m_id{other.m_id} {}

    Entity::Entity(Entity&& other): m_entityManager{other.m_entityManager},
                                    m_id{other.m_id} {

    }

    Entity& Entity::operator=(const Entity& other) {
        m_entityManager = other.m_entityManager;
        m_id = other.m_id;
        return *this;
    }


}
