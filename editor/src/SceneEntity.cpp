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

#include <robot2D/Ecs/EntityManager.hpp>
#include <editor/SceneEntity.hpp>
#include <editor/Components.hpp>
#include <editor/SceneGraph.hpp>

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

    void SceneEntity::addChild(SceneEntity& sceneEntity) {
        sceneEntity.m_parent = this;
        m_graph -> addEntityInternal(sceneEntity);

        auto& transformComponent = m_entity.getComponent<TransformComponent>();
        transformComponent.addChild(*this, sceneEntity);
    }

    bool SceneEntity::hasChildren() const {
        return m_entity.getComponent<TransformComponent>().hasChildren();
    }

    const std::list<SceneEntity>& SceneEntity::getChildren() const {
        return m_entity.getComponent<TransformComponent>().getChildren();
    }

    robot2D::FloatRect SceneEntity::calculateRect() const {
        const auto& tx = m_entity.getComponent<TransformComponent>();
        if(tx.getRotation() != 0.f)
            return getBoundingBox();

        robot2D::FloatRect rect;
        auto pos = tx.getPosition();
        auto size = tx.getSize();
        auto origin = tx.getOrigin();
        rect.lx = pos.x - size.x * origin.x;
        rect.ly = pos.y - size.y * origin.y;
        rect.width = size.x;
        rect.height = size.y;
        rect.setRotateAngle(tx.getRotation());
        return rect;
    }

    robot2D::FloatRect SceneEntity::getBoundingBox() const {
        const auto& tx = m_entity.getComponent<TransformComponent>();
        return tx.getGlobalBounds();
    }

    std::list<SceneEntity>& SceneEntity::getChildren() {
        return m_entity.getComponent<TransformComponent>().getChildren();
    }


} // namespace editor