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
#include <cmath>
#include <robot2D/Util/Logger.hpp>
#include <editor/physics/Box2DPhysicsAdapter.hpp>
#include <editor/Components.hpp>

namespace editor {
    namespace {
        constexpr float pixelsPerMeter = 32.f;
        constexpr float radiansPerDegree = 3.14159265358979323846f / 180.f;
        bool finite(float value) {
            return std::isfinite(value);
        }
        bool exitEvent(const PhysicsContact2D& e) {
            return e.type == PhysicsEventType::Exit || e.type == PhysicsEventType::ExitTrigger;
        }
        PhysicsContact2D exiting(PhysicsContact2D e) {
            e.type = e.isSensorA || e.isSensorB ? PhysicsEventType::ExitTrigger : PhysicsEventType::Exit;
            e.pointCount = 0; e.normalImpulse = 0.f; e.tangentImpulse = 0.f;
            return e;
        }
        b2Filter filterFor(const Collider2DComponent& c) {
            b2Filter filter;
            filter.categoryBits = c.filter.categoryBits;
            filter.maskBits = c.useDefaultMask ? phys2d::LayerRegistry::I().defaultMaskFor(
                phys2d::layerNameFromCategoryBit(c.filter.categoryBits)) : c.filter.maskBits;
            filter.groupIndex = c.filter.groupIndex;
            return filter;
        }
    }
    Box2DPhysicsAdapter::~Box2DPhysicsAdapter() {
        clearWorld();
    }
    Box2DPhysicsAdapter::PairKey Box2DPhysicsAdapter::key(const PhysicsContact2D& e) {
        return {e.fixtureA, e.fixtureB};
    }
    bool Box2DPhysicsAdapter::readContact(b2Contact* c, PhysicsContact2D& e) const {
        auto* a = static_cast<BodyEntry*>(c -> GetFixtureA() -> GetUserData().getData());
        auto* b = static_cast<BodyEntry*>(c -> GetFixtureB() -> GetUserData().getData());
        if(!a || !b || !a -> entity || !b -> entity) return false;
        e = PhysicsContact2D{};
        e.entityA = a -> id; e.entityB = b -> id;
        e.fixtureA = a -> fixtureID; e.fixtureB = b -> fixtureID;
        e.isSensorA = c -> GetFixtureA() -> IsSensor(); e.isSensorB = c -> GetFixtureB() -> IsSensor();
        const auto fa = c -> GetFixtureA() -> GetFilterData(), fb = c -> GetFixtureB() -> GetFilterData();
        e.categoryA = fa.categoryBits; e.categoryB = fb.categoryBits; e.maskA = fa.maskBits; e.maskB = fb.maskBits;
        e.type = e.isSensorA || e.isSensorB ? PhysicsEventType::EnterTrigger : PhysicsEventType::Enter;
        e.pointCount = static_cast<std::uint8_t>(c -> GetManifold() -> pointCount);
        if(e.pointCount) {
            b2WorldManifold manifold{}; c -> GetWorldManifold(&manifold);
            e.nx = manifold.normal.x; e.ny = manifold.normal.y;
            for(unsigned i = 0; i < e.pointCount; ++i)
                e.points[i] = {manifold.points[i].x, manifold.points[i].y, manifold.separations[i]};
        }
        // Stable orientation; the normal points A -> B, independently of Box2D ordering.
        if(e.entityB < e.entityA) {
            std::swap(e.entityA, e.entityB); std::swap(e.fixtureA, e.fixtureB);
            std::swap(e.categoryA, e.categoryB); std::swap(e.maskA, e.maskB);
            std::swap(e.isSensorA, e.isSensorB); e.nx = -e.nx; e.ny = -e.ny;
        }
        return true;
    }
    void Box2DPhysicsAdapter::BeginContact(b2Contact* c) {
        PhysicsContact2D e{};
        if(!readContact(c, e) || !m_contacts.emplace(key(e), e).second) return;
        m_entered.insert(key(e)); m_events.push_back(e);
    }
    void Box2DPhysicsAdapter::EndContact(b2Contact* c) {
        PhysicsContact2D e{}; if(!readContact(c, e)) return;
        const auto it = m_contacts.find(key(e)); if(it == m_contacts.end()) return;
        m_events.push_back(exiting(it -> second)); m_contacts.erase(it);
    }
    void Box2DPhysicsAdapter::PreSolve(b2Contact* c, const b2Manifold*) {
        PhysicsContact2D e{}; if(!readContact(c, e)) return;
        const auto it = m_contacts.find(key(e)); if(it != m_contacts.end()) it -> second = e;
    }
    void Box2DPhysicsAdapter::PostSolve(b2Contact* c, const b2ContactImpulse* impulse) {
        PhysicsContact2D e{}; if(!readContact(c, e)) return;
        const auto it = m_contacts.find(key(e)); if(it == m_contacts.end()) return;
        for(int i = 0; i < impulse -> count; ++i) {
            it -> second.normalImpulse += impulse -> normalImpulses[i];
            it -> second.tangentImpulse += impulse -> tangentImpulses[i];
        }
    }
    bool Box2DPhysicsAdapter::alive(std::uint64_t id) const {
        return !m_pendingRemovals.count(id) && std::any_of(m_bodies.begin(), m_bodies.end(), [id](const auto& b) {
            return b.id == id && static_cast<bool>(b.entity);
        });
    }
    void Box2DPhysicsAdapter::update(float dt) {
        if(!m_world || m_dispatching || !finite(dt) || dt <= 0.f) return;
        for(const auto& b: m_bodies) if(!b.entity) m_pendingRemovals.insert(b.id);
        flushOperations(); if(!m_world) return;
        for(auto& b: m_bodies) {
            if(!b.fixture || !b.entity.hasComponent<Collider2DComponent>()) continue;
            auto& c = b.entity.getComponent<Collider2DComponent>();
            if(c.markFilterDirty) {
                try { b.fixture -> SetFilterData(filterFor(c)); }
                catch(const std::exception& error) {
                    RB_EDITOR_WARN("[Physics] Filter rejected; entity={}; reason={}", b.id, error.what());
                }
                c.markFilterDirty = false;
            }
        }
        m_events = std::move(m_pendingEvents); m_pendingEvents.clear(); m_entered.clear();
        for(auto& [pair, e]: m_contacts) {
            e.normalImpulse = 0.f; e.tangentImpulse = 0.f;
        }
        m_world -> Step(dt, 6, 2);
        // Sensors and sleeping contacts do not necessarily receive PreSolve.
        for(auto* contact = m_world -> GetContactList(); contact; contact = contact -> GetNext()) {
            if(!contact -> IsTouching()) continue;
            PhysicsContact2D current{};
            if(!readContact(contact, current)) continue;
            const auto found = m_contacts.find(key(current));
            if(found == m_contacts.end()) continue;
            current.normalImpulse = found -> second.normalImpulse;
            current.tangentImpulse = found -> second.tangentImpulse;
            found -> second = current;
        }
        for(auto& b: m_bodies) updateEntity(b);
        for(auto& e: m_events) {
            const auto it = m_contacts.find(key(e)); if(!exitEvent(e) && it != m_contacts.end()) e = it -> second;
        }
        for(const auto& [pair, contact]: m_contacts) {
            if(m_entered.count(pair)) continue;
            auto e = contact;
            e.type = e.isSensorA || e.isSensorB ? PhysicsEventType::StayTrigger : PhysicsEventType::Stay;
            m_events.push_back(e);
        }
        auto events = std::move(m_events); m_events.clear(); auto callback = m_callback;
        m_dispatching = true;
        try {
            for(const auto& e: events) {
                if(m_stopRequested) break;
                if(!callback) continue;
                if(alive(e.entityA) && (exitEvent(e) || alive(e.entityB))) callback(e, UUID{e.entityA},
                        UUID{e.entityB});
                if(m_stopRequested) break;
                if(alive(e.entityB) && (exitEvent(e) || alive(e.entityA))) callback(e, UUID{e.entityB},
                        UUID{e.entityA});
            }
        }
        catch(...) { m_dispatching = false; flushOperations(); throw; }
        m_dispatching = false; flushOperations();
    }
    void Box2DPhysicsAdapter::updateEntity(BodyEntry& b) {
        if(!b.entity || !b.entity.hasComponent<TransformComponent>()) return;
        auto& t = b.entity.getComponent<TransformComponent>(); const auto p = b.body -> GetPosition();
        t.setPosition({p.x * pixelsPerMeter, p.y * pixelsPerMeter});
        t.setRotate(b.body -> GetAngle() / radiansPerDegree);
    }
    void Box2DPhysicsAdapter::start(editorEntityList& entities) {
        if(m_dispatching) {
            RB_EDITOR_WARN("[Physics] Start rejected; reason=callback dispatch in progress"); return;
        }
        clearWorld(); m_world = std::make_unique<b2World>(b2Vec2{0.f, 9.8f}); m_world -> SetContactListener(this);
        for(auto entity: entities) addEntity(entity);
    }
    void Box2DPhysicsAdapter::addEntity(SceneEntity entity) {
        if(!entity || !m_world) return;
        if(entity.hasComponent<Physics2DComponent>() && entity.hasComponent<TransformComponent>() &&
            !std::any_of(m_bodies.begin(), m_bodies.end(), [&](const auto& b) { return b.id == entity.getUUID(); })) {
            auto& t = entity.getComponent<TransformComponent>(); auto& p = entity.getComponent<Physics2DComponent>();
            const auto position = t.getPosition(), size = t.getSize();
            if(!finite(position.x) || !finite(position.y) || !finite(t.getRotation()) || !finite(p.gravityScale) ||
                !finite(p.linearDamping) || p.linearDamping < 0.f || !finite(p.angularDamping) ||
                p.angularDamping < 0.f) {
                RB_EDITOR_WARN("[Physics] Body rejected; entity={}; reason=invalid transform or body settings",
                    std::uint64_t(entity.getUUID()));
                return;
            }
            b2PolygonShape shape; b2Filter filter;
            if(entity.hasComponent<Collider2DComponent>()) {
                const auto& c = entity.getComponent<Collider2DComponent>(); const auto origin = t.getOrigin();
                const float hx = size.x * c.size.x / pixelsPerMeter, hy = size.y * c.size.y / pixelsPerMeter;
                if(!finite(hx) || !finite(hy) || hx <= b2_epsilon || hy <= b2_epsilon ||
                    !finite(c.offset.x) || !finite(c.offset.y) || !finite(origin.x) || !finite(origin.y) ||
                    !finite(c.density) || c.density < 0.f || !finite(c.friction) || c.friction < 0.f ||
                    !finite(c.restitution) || c.restitution < 0.f || !finite(c.restitutionThreshold) ||
                    c.restitutionThreshold < 0.f) {
                    RB_EDITOR_WARN("[Physics] Body rejected; entity={}; reason=invalid box geometry or material",
                        std::uint64_t(entity.getUUID()));
                    return;
                }
                try { filter = filterFor(c); }
                catch(const std::exception& error) {
                    RB_EDITOR_WARN("[Physics] Body rejected; entity={}; reason={}", std::uint64_t(entity.getUUID()),
                        error.what()); return;
                }
                shape.SetAsBox(hx, hy, {(size.x * (0.5f - origin.x) + c.offset.x) / pixelsPerMeter,
                                        (size.y * (0.5f - origin.y) + c.offset.y) / pixelsPerMeter}, 0.f);
            }
            b2BodyDef definition;
            switch(p.type) {
            case Physics2DComponent::BodyType::Static: definition.type = b2_staticBody; break;
            case Physics2DComponent::BodyType::Dynamic: definition.type = b2_dynamicBody; break;
            case Physics2DComponent::BodyType::Kinematic: definition.type = b2_kinematicBody; break;
            default: return;
            }
            definition.position.Set(position.x / pixelsPerMeter, position.y / pixelsPerMeter);
            definition.angle = t.getRotation() * radiansPerDegree; definition.fixedRotation = p.fixedRotation;
            definition.bullet = p.bullet; definition.gravityScale = p.gravityScale;
            definition.linearDamping = p.linearDamping; definition.angularDamping = p.angularDamping;
            auto* body = m_world -> CreateBody(&definition);
            m_bodies.push_back({entity, entity.getUUID(), body, nullptr, m_nextFixtureID++});
            auto& entry = m_bodies.back(); p.runtimeBody = body;
            if(entity.hasComponent<Collider2DComponent>()) {
                auto& c = entity.getComponent<Collider2DComponent>(); b2FixtureDef fixture;
                fixture.shape = &shape; fixture.isSensor = c.isTrigger; fixture.density = c.density;
                fixture.friction = c.friction; fixture.restitution = c.restitution;
                fixture.restitutionThreshold = c.restitutionThreshold; fixture.filter = filter;
                fixture.userData.setData(&entry); entry.fixture = body -> CreateFixture(&fixture);
                c.runtimeFixture = entry.fixture; c.markFilterDirty = false;
                c.userData = {entity.getUUID(), c.oneWay, c.oneWayNormal, true};
            }
        }
        for(auto child: entity.getChildren()) addEntity(child);
    }
    void Box2DPhysicsAdapter::removeEntity(std::uint64_t id) {
        const auto it = std::find_if(m_bodies.begin(), m_bodies.end(), [id](const auto& b) { return b.id == id; });
        if(it == m_bodies.end()) return;
        for(auto contact = m_contacts.begin(); contact != m_contacts.end();) {
            if(contact -> second.entityA == id || contact -> second.entityB == id) {
                m_pendingEvents.push_back(exiting(contact -> second)); contact = m_contacts.erase(contact);
            }
            else ++contact;
        }
        if(it -> entity.hasComponent<Physics2DComponent>()) it -> entity.getComponent<Physics2DComponent>().runtimeBody
                = nullptr;
        if(it -> entity.hasComponent<Collider2DComponent>()) it -> entity.getComponent<Collider2DComponent>().
            runtimeFixture = nullptr;
        m_world -> DestroyBody(it -> body); m_bodies.erase(it);
    }
    void Box2DPhysicsAdapter::flushOperations() {
        if(m_stopRequested) {
            clearWorld(); return;
        }
        auto removals = std::move(m_pendingRemovals); m_pendingRemovals.clear();
        for(auto id: removals) removeEntity(id);
        auto additions = std::move(m_pendingAdditions); m_pendingAdditions.clear();
        for(auto entity: additions) if(entity && !removals.count(entity.getUUID())) addEntity(entity);
    }
    void Box2DPhysicsAdapter::addRuntime(SceneEntity entity) {
        if(m_dispatching) m_pendingAdditions.push_back(entity); else addEntity(entity);
    }
    void Box2DPhysicsAdapter::removeRuntime(SceneEntity entity) {
        if(!entity) return;
        m_pendingRemovals.insert(entity.getUUID());
        for(auto child: entity.getChildren()) removeRuntime(child);
        if(!m_dispatching && m_world) flushOperations();
    }
    void Box2DPhysicsAdapter::clearWorld() {
        if(m_world) m_world -> SetContactListener(nullptr);
        for(auto& b: m_bodies) {
            if(b.entity.hasComponent<Physics2DComponent>()) b.entity.getComponent<Physics2DComponent>().runtimeBody =
                    nullptr;
            if(b.entity.hasComponent<Collider2DComponent>()) b.entity.getComponent<Collider2DComponent>().runtimeFixture
                    = nullptr;
        }
        m_world.reset(); m_bodies.clear(); m_contacts.clear(); m_entered.clear(); m_events.clear();
        m_pendingEvents.clear(); m_pendingAdditions.clear(); m_pendingRemovals.clear();
        m_callback = {}; m_nextFixtureID = 1; m_stopRequested = false;
    }
    void Box2DPhysicsAdapter::stop() {
        if(m_dispatching) m_stopRequested = true; else clearWorld();
    }
}
