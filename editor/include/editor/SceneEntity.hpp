#pragma once
#include <robot2D/Ecs/Entity.hpp>
#include "Uuid.hpp"

namespace editor {
    class SceneEntity {
    public:
        SceneEntity() = default;

        SceneEntity(robot2D::ecs::Entity&& entity);
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
        explicit operator bool() const noexcept {
            if(!m_entity)
                return false;
            return !m_entity.destroyed();
        }

        void addChild(const SceneEntity& sceneEntity);
        [[nodiscard]]
        UUID getUUID() const;

        bool hasChildren() const;

        robot2D::ecs::Entity getWrappedEntity() const { return m_entity; }
        const std::vector<robot2D::ecs::Entity>& getChildren() const;
    private:
        robot2D::ecs::Entity m_entity;
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