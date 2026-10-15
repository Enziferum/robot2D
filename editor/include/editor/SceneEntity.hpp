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

#include <vector>
#include <list>
#include <optional>
#include <cassert>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Graphics/Rect.hpp>
#include "Uuid.hpp"

namespace editor {
    class Scene;
    enum class SceneContext {
        Editor,
        Runtime
    };
    // A lightweight ECS descriptor. Its owning scene must outlive all copies.
    class SceneEntity {
    public:
        SceneEntity() = default;

        explicit SceneEntity(robot2D::ecs::Entity&& entity);
        explicit SceneEntity(const robot2D::ecs::Entity& entity);
        SceneEntity(const SceneEntity& other) = default;
        SceneEntity& operator=(const SceneEntity& other) = default;
        SceneEntity(SceneEntity&& other) = default;
        SceneEntity& operator=(SceneEntity&& other) = default;
        ~SceneEntity() = default;

        template<typename T>
        bool hasComponent() const;

        template<typename T>
        T& getComponent();

        template<typename T>
        const T& getComponent() const;

        template<typename T, typename ...Args>
        T& addComponent(Args&& ... args);

        template<typename T>
        void removeComponent();

        friend bool operator==(const SceneEntity& left, const SceneEntity& right);
        friend bool operator!=(const SceneEntity& left, const SceneEntity& right);
        explicit operator bool() const noexcept;

        bool addChild(SceneEntity sceneEntity);
        [[nodiscard]]
        UUID getUUID() const;

        bool hasChildren() const;

        robot2D::ecs::Entity getWrappedEntity() const { return m_entity; }
        std::vector<SceneEntity> getChildren() const;


        SceneEntity getParent() const;

        /// \brief usefull for QuadTree insertion
        robot2D::FloatRect calculateRect() const;
        robot2D::FloatRect getBoundingBox() const;

        bool isChild() const;

        const std::string& getName() const;
    private:
        friend class Scene;

        robot2D::ecs::Entity m_entity;
        Scene* m_scene { nullptr };
        SceneContext m_context = SceneContext::Editor;
        std::uint64_t m_contextVersion { 0 };
        std::optional<UUID> m_uuid;
    };

    template<typename T>
    inline bool SceneEntity::hasComponent() const {
        return static_cast<bool>(*this) && m_entity.hasComponent<T>();
    }

    template<typename T>
    T& SceneEntity::getComponent() {
        assert(static_cast<bool>(*this) && "Invalid SceneEntity");
        return m_entity.getComponent<T>();
    }

    template<typename T>
    const T& SceneEntity::getComponent() const {
        assert(static_cast<bool>(*this) && "Invalid SceneEntity");
        return m_entity.getComponent<T>();
    }

    template<typename T, typename ...Args>
    T& SceneEntity::addComponent(Args&& ... args) {
        assert(static_cast<bool>(*this) && "Invalid SceneEntity");
        return m_entity.addComponent<T>(std::forward<Args>(args)...);
    }


    template<typename T>
    void SceneEntity::removeComponent() {
        assert(static_cast<bool>(*this) && "Invalid SceneEntity");
        m_entity.removeComponent<T>();
    }

} // namespace editor
