#pragma once
#include <robot2D/Ecs/Entity.hpp>
#include "Uuid.hpp"

namespace editor {
    class SceneEntity {
    public:
        SceneEntity();
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

    template<typename T>
    void SceneEntity::removeComponent() {
        m_entity.removeComponent<T>();
    }

} // namespace editor