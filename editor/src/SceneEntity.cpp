#include <editor/SceneEntity.hpp>
#include <editor/Components.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

namespace editor {

    SceneEntity::SceneEntity(robot2D::ecs::Entity&& entity): m_entity{std::move(entity)} {}

    SceneEntity::SceneEntity(const robot2D::ecs::Entity& entity): m_entity(entity) {}

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
        transformComponent.addChild(SceneEntity(m_entity), sceneEntity);
    }

    bool SceneEntity::hasChildren() const {
        return m_entity.getComponent<TransformComponent>().hasChildren();
    }

    const std::vector<SceneEntity>& SceneEntity::getChildren() const {
        return m_entity.getComponent<TransformComponent>().getChildren();
    }



} // namespace editor