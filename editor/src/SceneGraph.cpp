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

#include <editor/SceneGraph.hpp>
#include <editor/Components.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

namespace editor {

    SceneGraph::SceneGraph(robot2D::MessageBus& messageBus): m_scene{messageBus} {}

    void SceneGraph::update(float dt) {
        m_deletePendingEntities.swap(m_deletePendingBuffer);
        for (auto& entity : m_deletePendingEntities) {
//            for (auto child : entity.getChildren())
//                child.removeSelf();

            m_sceneEntities.erase(std::remove_if(m_sceneEntities.begin(), m_sceneEntities.end(),
                [&entity](const SceneEntity& item) {
                    return item == entity;
                }), m_sceneEntities.end());

            m_scene.removeEntity(entity.getWrappedEntity());
        }
        m_deletePendingEntities.clear();


    }

    SceneEntity SceneGraph::createEntity(robot2D::ecs::Entity&& entity) {
        return SceneEntity{ entity };
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

    bool SceneGraph::cloneSelf(SceneGraph& cloneGraph) {
        return false;
    }


} // namespace editor