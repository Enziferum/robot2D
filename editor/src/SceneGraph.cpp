#include <editor/SceneGraph.hpp>
#include <editor/Components.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

namespace editor {

    SceneEntity SceneGraph::createEntity(robot2D::ecs::Entity&& entity) {
        return SceneEntity{std::move(entity)};
    }

    void SceneGraph::addEntity(SceneEntity&& sceneEntity) {
        m_sceneEntities.emplace_back(sceneEntity);
    }

    SceneEntity SceneGraph::getEntity(UUID uuid) const {
        auto start = m_sceneEntities.begin();
        for(; start != m_sceneEntities.end(); ++start) {
            auto entity = *start;
            if(entity.getUUID() == uuid)
                return entity;

            if(entity.hasChildren()) {
                auto childEntity = getEntityChild(entity, uuid);
                if(childEntity)
                    return childEntity;
            }
        }

        return {};
    }

    SceneEntity SceneGraph::getEntityChild(SceneEntity& parent, UUID uuid) const {
        for(auto& child: parent.getChildren()) {
            if(child.getComponent<IDComponent>().ID == uuid) {
                auto sceneChild = child;
                return SceneEntity(std::move(sceneChild));
            }

            auto& tx = child.getComponent<TransformComponent>();
            if(tx.hasChildren()) {
                auto sceneChild = child;
                auto childParent = SceneEntity(std::move(sceneChild));
                auto childEntity = getEntityChild(childParent, uuid);
                if(childEntity)
                    return childEntity;
            }
        }

        return {};
    }


} // namespace editor