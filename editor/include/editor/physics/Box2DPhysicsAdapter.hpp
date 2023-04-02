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
        void registerCallback(PhysicsCallbackType callbackType, editor::PhysicsCallback&& callback) override {
            m_callbacks[callbackType] = std::move(callback);
        }
    private:
        std::unique_ptr<b2World> m_physicsWorld{nullptr};
        editorEntityList* m_entityList{nullptr};
        std::unordered_map<PhysicsCallbackType, PhysicsCallback> m_callbacks;
    };
}