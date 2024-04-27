#include <editor/SceneGraph.hpp>
#include <editor/Components.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

namespace editor {

    void SceneGraph::updateSelf(robot2D::ecs::Scene& ecsScene) {
        m_deletePendingEntities.swap(m_deletePendingBuffer);
        for (auto& entity : m_deletePendingEntities) {
            for (auto child : entity.getChildren())
                child.removeSelf();

            m_sceneEntities.erase(std::remove_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                [&entity](const SceneEntity& item) {
                    return item == entity;
                }), m_sceneEntities.end());

            ecsScene.removeEntity(entity.getWrappedEntity());
        }
        m_deletePendingEntities.clear();
    }

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


    bool SceneGraph::setBefore(const SceneEntity& source, const SceneEntity& target) {

        /// 1. if level not equal don't do

        // const auto& parent = source.getParent();
        



        //auto sourceIter = std::find_if(m_sceneEntities.begin(), m_sceneEntities.end(),
        //    [&source](robot2D::ecs::Entity item) {
        //        return item.getIndex() == source.getIndex();
        //    });

        //auto targetIter = std::find_if(m_sceneEntities.begin(), m_sceneEntities.end(),
        //    [&target](robot2D::ecs::Entity item) {
        //        return item.getIndex() == target.getIndex();
        //    });

        //auto sourceOldDistance = std::distance(m_sceneEntities.begin(), sourceIter);
        //auto targetOldDistance = std::distance(m_sceneEntities.begin(), targetIter);

        //if (sourceOldDistance > targetOldDistance) {
        //    /// insert before remove last
        //    m_insertItems.push_back(std::make_tuple(targetIter, source, ReorderDeleteType::Last));
        //}
        //else if (sourceOldDistance < targetOldDistance) {
        //    m_insertItems.push_back(std::make_tuple(targetIter, source, ReorderDeleteType::First));
        //}

        return true;
    }


    void SceneGraph::removeEntity(const SceneEntity& entity) {
        m_deletePendingBuffer.push_back(entity);
    }



} // namespace editor