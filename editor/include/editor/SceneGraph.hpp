#pragma once
#include <list>

#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Ecs/Scene.hpp>
#include "SceneEntity.hpp"

namespace editor {


    class SceneGraph {
    public:
        SceneGraph() = default;
        ~SceneGraph() = default;
        SceneGraph(const SceneGraph& other) = delete;
        SceneGraph& operator=(const SceneGraph& other) = delete;
        SceneGraph(SceneGraph&& other) = delete;
        SceneGraph& operator=(SceneGraph&& other) = delete;

        void updateSelf(robot2D::ecs::Scene& ecsScene);

        SceneEntity createEntity(robot2D::ecs::Entity&& entity);
        void addEntity(SceneEntity&& sceneEntity);
        SceneEntity getEntity(UUID uuid) const;
        void removeEntity(const SceneEntity& entity);


        bool setBefore(const SceneEntity& source, const SceneEntity& target);

        const std::list<SceneEntity>& getEntities() const { return m_sceneEntities; }
        std::list<SceneEntity>& getEntities() { return m_sceneEntities; }
    private:
        SceneEntity getEntityChild(SceneEntity& parent, UUID uuid) const;
    private:
        std::list<SceneEntity> m_sceneEntities;

        /// <summary>
        ///  double buffer pattern for removing 
        /// </summary>
        std::vector<SceneEntity> m_deletePendingEntities;
        std::vector<SceneEntity> m_deletePendingBuffer;
    };



} // namespace editor