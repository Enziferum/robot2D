/*********************************************************************
(c) Alex Raag 2026
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
#include <editor/Scene.hpp>

namespace editor {

    SceneEntity::SceneEntity(robot2D::ecs::Entity&& entity): m_entity{std::move(entity)} {}

    SceneEntity::SceneEntity(const robot2D::ecs::Entity& entity): m_entity(entity) {}

    bool operator==(const SceneEntity& left, const SceneEntity& right) {
        return left.m_scene == right.m_scene && left.m_context == right.m_context
               && left.m_contextVersion == right.m_contextVersion && left.m_entity == right.m_entity;
    }

    bool operator!=(const SceneEntity& left, const SceneEntity& right) {
        return !(left == right);
    }

    SceneEntity::operator bool() const noexcept {
        if(m_scene && !m_scene -> isContextValid(m_context, m_contextVersion))
            return false;
        return static_cast<bool>(m_entity) && !m_entity.destroyed();
    }

    void SceneEntity::markModified() const {
        if(*this && m_scene && m_context == SceneContext::Editor)
            m_scene -> markModified();
    }

    UUID SceneEntity::getUUID() const {
        if(m_uuid)
            return *m_uuid;
        if(!hasComponent<IDComponent>())
            return UUID{0};
        return m_entity.getComponent<IDComponent>().ID;
    }

    bool SceneEntity::addChild(SceneEntity entity) {
        return m_scene && m_scene -> reparent(entity, *this);
    }

    bool SceneEntity::hasChildren() const {
        return !getChildren().empty();
    }

    std::vector<SceneEntity> SceneEntity::getChildren() const {
        return m_scene ? m_scene -> getChildren(*this) : std::vector<SceneEntity>{};
    }

    SceneEntity SceneEntity::getParent() const {
        return m_scene ? m_scene -> getParent(*this) : SceneEntity{};
    }

    bool SceneEntity::isChild() const {
        return static_cast<bool>(getParent());
    }

    robot2D::FloatRect SceneEntity::calculateRect() const {
        const auto& tx = getComponent<TransformComponent>();
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
        const auto& tx = getComponent<TransformComponent>();
        return tx.getGlobalBounds();
    }

    const std::string& SceneEntity::getName() const {
        return getComponent<TagComponent>().getTag();
    }


} // namespace editor
