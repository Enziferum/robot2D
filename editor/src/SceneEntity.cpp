#include <editor/SceneEntity.hpp>
#include <editor/Components.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

namespace editor {

    SceneEntity::SceneEntity() {}

    bool operator==(const SceneEntity& left, const SceneEntity& right) {
        return (left.m_entity == right.m_entity) && (left.getUUID() == right.getUUID());
    }

    UUID SceneEntity::getUUID() const {
        auto& idComponent = m_entity.getComponent<IDComponent>();
        return idComponent.ID;
    }

    void SceneEntity::addChild(const SceneEntity& sceneEntity) {
        auto& transformComponent = m_entity.getComponent<TransformComponent>();
        transformComponent.addChild(m_entity, sceneEntity.m_entity);
    }

} // namespace editor