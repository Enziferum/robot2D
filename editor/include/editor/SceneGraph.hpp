#pragma once
#include <list>

#include <robot2D/Ecs/Entity.hpp>
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

        SceneEntity createEntity(robot2D::ecs::Entity&& entity);
        void addEntity(SceneEntity&& sceneEntity);
        SceneEntity getEntity(UUID uuid) const;

        bool setBefore(const SceneEntity& source, const SceneEntity& target);

        const std::list<SceneEntity>& getEntities() const { return m_sceneEntities; }
        std::list<SceneEntity>& getEntities() { return m_sceneEntities; }
    private:
        SceneEntity getEntityChild(SceneEntity& parent, UUID uuid) const;
    private:
        std::list<SceneEntity> m_sceneEntities;
    };



} // namespace editor