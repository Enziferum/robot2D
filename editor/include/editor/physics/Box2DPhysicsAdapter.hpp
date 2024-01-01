/*********************************************************************
(c) Alex Raag 2023
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

#include <memory>
#include <box2d/box2d.h>
#include "IPhysics2DAdapter.hpp"

namespace editor {
    class Box2DPhysicsAdapter final: public IPhysics2DAdapter, b2ContactListener {
    public:
        Box2DPhysicsAdapter() = default;
        ~Box2DPhysicsAdapter() override = default;

        /// box2d API
        void BeginContact(b2Contact* contact) override;
        void EndContact(b2Contact* contact) override;
        void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
        void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

        /// IPhysics2DAdapter Api
        void update(float dt) override;
        void start(editorEntityList& entityList) override;
        void stop() override;
        void addRuntime(robot2D::ecs::Entity entity) override;
        void registerCallback(PhysicsCallbackType callbackType, editor::PhysicsCallback&& callback) override {
            m_callbacks[callbackType] = std::move(callback);
        }
    private:
        void addEntity(robot2D::ecs::Entity entity);
        void updateEntity(robot2D::ecs::Entity entity);
    private:
        std::unique_ptr<b2World> m_physicsWorld{nullptr};
        std::list<robot2D::ecs::Entity> m_entityList;
        std::unordered_map<PhysicsCallbackType, PhysicsCallback> m_callbacks;
    };
}