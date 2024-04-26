#include <editor/SceneEntity.hpp>
#include <editor/Components.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

namespace editor {

    SceneEntity::SceneEntity(robot2D::ecs::Entity&& entity): m_entity{std::move(entity)} {}

    bool operator==(const SceneEntity& left, const SceneEntity& right) {
        return (left.m_entity == right.m_entity) && (left.getUUID() == right.getUUID());
    }

    bool operator!=(const SceneEntity& left, const SceneEntity& right) {
        return !(left == right);
    }

    UUID SceneEntity::getUUID() const {
        auto& idComponent = m_entity.getComponent<IDComponent>();
        return idComponent.ID;
    }

    void SceneEntity::addChild(const SceneEntity& sceneEntity) {
        auto& transformComponent = m_entity.getComponent<TransformComponent>();
        transformComponent.addChild(m_entity, sceneEntity.m_entity);
    }

    bool SceneEntity::hasChildren() const {
        return m_entity.getComponent<TransformComponent>().hasChildren();
    }

    const std::vector<robot2D::ecs::Entity>& SceneEntity::getChildren() const {
        return m_entity.getComponent<TransformComponent>().getChildren();
    }

} // namespace editor