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
#include <string>
#include "Defines.hpp"

namespace robot2D::ecs {
    class EntityManager;

    class Entity {
    public:
        Entity();
        ~Entity() = default;

        template<typename T, typename ...Args>
        T& addComponent(Args&& ... args);

        template<typename T>
        bool hasComponent();

        template<typename T>
        T& getComponent();

        template<typename T>
        const T& getComponent() const;

        template<typename T>
        void removeComponent();

        EntityID getIndex() const { return m_id; }

        Bitmask getComponentMask() const;

        void setTag(const std::string& tag) { m_tag = tag; }

        const std::string& getTag() const { return m_tag; }

        std::string& getTag() { return m_tag; }

        friend bool operator == (const Entity& l, const Entity& r);

        operator bool() {
            return m_entityManager != nullptr && m_id != -1;
        }
    private:
        friend class EntityManager;
        explicit Entity(EntityManager* entityManager, const EntityID& id);
        EntityManager* m_entityManager;

        EntityID m_id;
        std::string m_tag;
    };

}
