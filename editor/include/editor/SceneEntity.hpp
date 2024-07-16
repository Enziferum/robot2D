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

#include <vector>
#include <list>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Graphics/Rect.hpp>
#include "Uuid.hpp"

namespace editor {
    class SceneGraph;
    class SceneEntity {
    public:
        SceneEntity() = default;

        explicit SceneEntity(robot2D::ecs::Entity&& entity);
        explicit SceneEntity(const robot2D::ecs::Entity& entity);
        SceneEntity(const SceneEntity& other) = default;
        SceneEntity& operator=(const SceneEntity& other) = default;
        SceneEntity(SceneEntity&& other) = default;
        SceneEntity& operator=(SceneEntity&& other) = default;
        ~SceneEntity() = default;

        template<typename T>
        bool hasComponent() const;

        template<typename T>
        T& getComponent();

        template<typename T>
        const T& getComponent() const;

        template<typename T, typename ...Args>
        T& addComponent(Args&& ... args);

        template<typename T>
        void removeComponent();

        friend bool operator==(const SceneEntity& left, const SceneEntity& right);
        friend bool operator!=(const SceneEntity& left, const SceneEntity& right);
        explicit operator bool() const noexcept {
            if(!m_entity)
                return false;
            return !m_entity.destroyed();
        }

        void addChild(SceneEntity& sceneEntity);
        [[nodiscard]]
        UUID getUUID() const;

        bool hasChildren() const;

        robot2D::ecs::Entity getWrappedEntity() const { return m_entity; }
        const std::list<SceneEntity>& getChildren() const;
        std::list<SceneEntity>& getChildren();


        const SceneEntity& getParent() const { return *m_parent; }
        SceneEntity* getParent() { return m_parent; }

        /// \brief usefull for QuadTree insertion
        robot2D::FloatRect calculateRect() const;
        robot2D::FloatRect getBoundingBox() const;

        bool isChild() const { return m_parent != nullptr; }
    private:
        friend class SceneGraph;

        robot2D::ecs::Entity m_entity;
        SceneEntity* m_parent { nullptr };
        SceneGraph* m_graph { nullptr };
    };

    template<typename T>
    inline bool SceneEntity::hasComponent() const {
        return m_entity.hasComponent<T>();
    }

    template<typename T>
    T& SceneEntity::getComponent() {
        return m_entity.getComponent<T>();
    }

    template<typename T>
    const T& SceneEntity::getComponent() const {
        return m_entity.getComponent<T>();
    }

    template<typename T, typename ...Args>
    T& SceneEntity::addComponent(Args&& ... args) {
        return m_entity.addComponent<T>(std::forward<Args>(args)...);
    }


    template<typename T>
    void SceneEntity::removeComponent() {
        m_entity.removeComponent<T>();
    }

} // namespace editor