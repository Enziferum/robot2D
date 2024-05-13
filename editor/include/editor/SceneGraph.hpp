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
#include <list>
#include <unordered_map>

#include <robot2D/Ecs/Scene.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Ecs/Scene.hpp>
#include "SceneEntity.hpp"

namespace editor {

    template<typename T>
    class sorted_vector {
    public:
        void push_back(const T& value) {}
    };


    class SceneGraph {
    private:
        using EntityContainer = std::vector<SceneEntity>;
    public:
        explicit SceneGraph(robot2D::MessageBus& messageBus);
        ~SceneGraph() = default;
        SceneGraph(const SceneGraph& other) = delete;
        SceneGraph& operator=(const SceneGraph& other) = delete;
        SceneGraph(SceneGraph&& other) = delete;
        SceneGraph& operator=(SceneGraph&& other) = delete;

        bool cloneSelf(SceneGraph& cloneGraph);
        void update(float dt);

        SceneEntity createEntity(robot2D::ecs::Entity&& entity);
        void addEntity(SceneEntity sceneEntity);


        SceneEntity getEntity(UUID uuid) const;
        void removeEntity(const SceneEntity& entity);


        bool setBefore(SceneEntity& source, SceneEntity& target);

        const std::list<SceneEntity>& getEntities() const { return m_sceneEntities; }
        std::list<SceneEntity>& getEntities() { return m_sceneEntities; }


        template<typename Component, typename Container>
        void filterEntities(Container& container) {
            for(auto& entity: m_sceneEntities) {
                if(entity.hasComponent<Component>())
                    container.push_back(entity);
                if(entity.hasChildren())
                    filterEntitiesChildren<Component>(container, entity);
            }

        }

    private:
        void addEntityInternal(SceneEntity sceneEntity);
        template<typename Component, typename Container>

        void filterEntitiesChildren(Container& container, const SceneEntity& parent) {
            for(auto& entity: parent.getChildren()) {
                if(entity.hasComponent<Component>())
                    container.push_back(entity);
                if(entity.hasChildren())
                    filterEntitiesChildren<Component>(container, entity);
            }
        }
    private:
        friend class SceneEntity;
        robot2D::ecs::Scene m_scene;

        std::list<SceneEntity> m_sceneEntities;
        /// \brief allowing to easy find by UUID ( need for fast search using by outside using from scripting engine).
        std::unordered_map<UUID, SceneEntity> m_AllSceneEntitiesMap;

        using ReorderIterator = std::list<SceneEntity>::const_iterator;
        struct {
            ReorderIterator targetIterator;
            ReorderIterator sourceIterator;
            SceneEntity sceneEntity;
            SceneEntity* childParent { nullptr };
            bool hasValues { false };
        } m_reorderInfo;

        /// <summary>
        ///  double buffer pattern for removing 
        /// </summary>
        std::vector<SceneEntity> m_deletePendingEntities;
        std::vector<SceneEntity> m_deletePendingBuffer;
    };



} // namespace editor