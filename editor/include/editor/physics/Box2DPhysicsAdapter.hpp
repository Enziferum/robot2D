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
#include <map>
#include <set>
#include <vector>
#include <box2d/box2d.h>
#include "IPhysics2DAdapter.hpp"

namespace editor {
    class Box2DPhysicsAdapter final : public IPhysics2DAdapter, private b2ContactListener {
    public:
        Box2DPhysicsAdapter() = default;
        Box2DPhysicsAdapter(const Box2DPhysicsAdapter&) = delete;
        Box2DPhysicsAdapter& operator=(const Box2DPhysicsAdapter&) = delete;
        Box2DPhysicsAdapter(Box2DPhysicsAdapter&&) = delete;
        Box2DPhysicsAdapter& operator=(Box2DPhysicsAdapter&&) = delete;
        ~Box2DPhysicsAdapter() override;

        void update(float dt) override;
        void start(editorEntityList& entities) override;
        void stop() override;
        void addRuntime(SceneEntity entity) override;
        void removeRuntime(SceneEntity entity) override;
        void registerCallback(PhysicsCallback&& callback) override {
            m_callback = std::move(callback);
        }
    private:
        struct BodyEntry {
            SceneEntity entity;
            std::uint64_t id;
            b2Body* body;
            b2Fixture* fixture;
            std::uint64_t fixtureID;
        };
        using PairKey = std::pair<std::uint64_t, std::uint64_t>;
        static PairKey key(const PhysicsContact2D& event);
        bool readContact(b2Contact* contact, PhysicsContact2D& event) const;
        void BeginContact(b2Contact* contact) override;
        void EndContact(b2Contact* contact) override;
        void PreSolve(b2Contact* contact, const b2Manifold*) override;
        void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;
        void addEntity(SceneEntity entity);
        void removeEntity(std::uint64_t id);
        void flushOperations();
        void clearWorld();
        bool alive(std::uint64_t id) const;
        void updateEntity(BodyEntry& entry);

        std::unique_ptr<b2World> m_world;
        std::list<BodyEntry> m_bodies;
        PhysicsCallback m_callback;
        std::map<PairKey, PhysicsContact2D> m_contacts;
        std::set<PairKey> m_entered;
        std::vector<PhysicsContact2D> m_events;
        std::vector<PhysicsContact2D> m_pendingEvents;
        std::vector<SceneEntity> m_pendingAdditions;
        std::set<std::uint64_t> m_pendingRemovals;
        std::uint64_t m_nextFixtureID{1};
        bool m_dispatching{false};
        bool m_stopRequested{false};
    };
}
