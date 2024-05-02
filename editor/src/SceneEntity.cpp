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