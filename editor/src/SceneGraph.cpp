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
#include <algorithm>

#include <editor/SceneGraph.hpp>
#include <editor/Components.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

namespace editor {

    SceneGraph::SceneGraph(robot2D::MessageBus& messageBus): m_scene{messageBus} {}

    void SceneGraph::update(float dt, robot2D::ecs::Scene& ecsScene) {
        m_deletePendingEntities.swap(m_deletePendingBuffer);
        for (auto& entity : m_deletePendingEntities) {
            for (auto child : entity.getChildren())
                child.getWrappedEntity().removeSelf();

            m_sceneEntities.erase(std::remove_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                [&entity](const SceneEntity& item) {
                    return item == entity;
                }), m_sceneEntities.end());
            ecsScene.removeEntity(entity.getWrappedEntity());
        }
        m_deletePendingEntities.clear();

        if(m_reorderInfo.hasValues) {
            if(m_reorderInfo.childParent) {
                auto& childList = m_reorderInfo.childParent -> getChildren();
                childList.insert(m_reorderInfo.targetIterator, m_reorderInfo.sceneEntity);
                childList.erase(m_reorderInfo.sourceIterator);
            }
            else {
                m_sceneEntities.insert(m_reorderInfo.targetIterator, m_reorderInfo.sceneEntity);
                m_sceneEntities.erase(m_reorderInfo.sourceIterator);
            }
            m_reorderInfo.hasValues = false;
        }
    }

    SceneEntity SceneGraph::createEntity(robot2D::ecs::Entity&& entity) {
        auto sceneEntity = SceneEntity{ std::move(entity) };
        sceneEntity.m_graph = this;
        return sceneEntity;
    }

    void SceneGraph::addEntity(SceneEntity sceneEntity) {
        auto back = m_sceneEntities.emplace_back(sceneEntity);
        m_AllSceneEntitiesMap[sceneEntity.getUUID()] = back;
    }

    SceneEntity SceneGraph::getEntity(UUID uuid) const {
        if(m_AllSceneEntitiesMap.find(uuid) != m_AllSceneEntitiesMap.end()) {
            auto pSceneEntity = m_AllSceneEntitiesMap.at(uuid);
            if(pSceneEntity)
                return pSceneEntity;
        }

        return {};
    }


    bool SceneGraph::setBefore(SceneEntity& source, SceneEntity& target) {
        if(source.isChild() && target.isChild()) {
            if(source.getParent() == target.getParent()) {
                auto* parent = source.getParent();
                const auto& childEntities = parent -> getChildren();

                auto sourceIter = std::find_if(childEntities.begin(), childEntities.end(),
                                               [&source](const SceneEntity& item) {
                                                   return item == source;
                                               });

                auto targetIter = std::find_if(childEntities.begin(), childEntities.end(),
                                               [&target](const SceneEntity& item) {
                    return item == target;
                });

                m_reorderInfo.targetIterator = targetIter;
                m_reorderInfo.sourceIterator = sourceIter;
                m_reorderInfo.sceneEntity = source;
                m_reorderInfo.hasValues = true;
                m_reorderInfo.childParent = parent;
                return true;
            }
        }
        else if(!source.isChild() && !target.isChild()) {
            auto sourceIter =
                    std::find_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                [&source](const SceneEntity& item) {
                    return item == source;
                });

            auto targetIter =
                    std::find_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                                 [&target](const SceneEntity& item) {
                return item == target;
            });


            m_reorderInfo.targetIterator = targetIter;
            m_reorderInfo.sourceIterator = sourceIter;
            m_reorderInfo.sceneEntity = source;
            m_reorderInfo.hasValues = true;
            return true;
        }

        return false;
    }


    void SceneGraph::removeEntity(const SceneEntity& entity) {
        m_deletePendingBuffer.push_back(entity);
    }

    bool SceneGraph::cloneSelf(SceneGraph& cloneGraph) {
        //constexpr bool cloneSystems = true;
        //cloneGraph.m_scene.cloneSelf(m_scene, cloneSystems);
        cloneGraph.m_sceneEntities = m_sceneEntities;
        cloneGraph.m_AllSceneEntitiesMap = m_AllSceneEntitiesMap;

        return false;
    }

    void SceneGraph::addEntityInternal(SceneEntity sceneEntity) {
        if(!sceneEntity.hasComponent<IDComponent>())
            return;
        m_AllSceneEntitiesMap[sceneEntity.getUUID()] = sceneEntity;
    }

    void SceneGraph::traverseGraph(TraverseFunction&& traverseFunction) {
        for(auto& entity: m_sceneEntities) {
            if(!entity)
                continue;
            traverseFunction(entity);
            if(entity.hasChildren())
                traverseGraphChildren(traverseFunction, entity);
        }
    }

    void SceneGraph::traverseGraphChildren(TraverseFunction& traverseFunction, SceneEntity parent) {
        for(auto& entity: parent.getChildren()) {
            if(!entity)
                continue;
            traverseFunction(entity);
            if(entity.hasChildren())
                traverseGraphChildren(traverseFunction, entity);
        }
    }


} // namespace editor