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

#include <robot2D/Util/Logger.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/physics/Box2DPhysicsAdapter.hpp>
#include <editor/physics/Layers2D.hpp>
#include <editor/Components.hpp>

namespace editor {

    void ApplyFilterFromUI(b2FixtureDef& fd,
                           const std::string& layerName,
                           const std::vector<std::string>& collidesWith, // пусто => дефолт
                           int16_t groupIndex,
                           bool useDefaultMask)
    {
        phys2d::FilterBits fb = useDefaultMask
                                ? phys2d::makeFilterDefault(layerName, groupIndex)
                                : phys2d::makeFilterCustomMask(layerName, collidesWith, groupIndex);

        fd.filter.categoryBits = fb.categoryBits;
        fd.filter.maskBits     = fb.maskBits;
        fd.filter.groupIndex   = fb.groupIndex;
    }

    namespace {
        constexpr int velocityUpdateIterations = 6;
        constexpr int positionUpdateIterations = 2;

        constexpr double PIXELS_PER_METERS = 32.0;
        constexpr double PI = 3.14159265358979323846;

        template<typename T>
        constexpr T pixel_to_meters(const T& x)
        {
            return x / PIXELS_PER_METERS;
        };

        template<typename T>
        constexpr T meters_to_pixels(const T& x)
        {
            return x*PIXELS_PER_METERS;
        }
    }

    struct PairKeySimple {
        uint64_t lo, hi;
        bool operator==(const PairKeySimple& o) const { return lo==o.lo && hi==o.hi; }
    };
    struct PairKeySimpleHash {
        size_t operator()(const PairKeySimple& k) const {
            return (size_t)k.lo*1315423911u ^ (size_t)k.hi;
        }
    };

// --- Правила, которыми управляет C#
    struct IgnorePairRule { double untilSec = 0.0; };
    struct IgnoreWithLayerRule { uint16_t layerMask = 0; double untilSec = 0.0; };

// --- Таблицы правил
    static std::unordered_map<PairKeySimple, IgnorePairRule, PairKeySimpleHash> g_ignorePairs;
    static std::unordered_map<uint64_t, IgnoreWithLayerRule> g_ignoreWithLayer;

// --- UserData на телах (для drop-through)
    struct BodyUD {
        uint64_t entityId = 0;
        double   dropThroughUntil = 0.0;
    };
    static inline BodyUD* GetBodyUD(const b2Body* b) {
        return (BodyUD*)b->GetUserData().pointer;
    }

// --- Вспомогалки
    static inline PairKeySimple MakePair(uint64_t a, uint64_t b) {
        return (a < b) ? PairKeySimple{a,b} : PairKeySimple{b,a};
    }




    struct ContactShapeData {
        UUID entityUuid;
        UUID fixtureUuid; /// optional just now
        std::uint16_t category;
        std::uint16_t mask;
        bool isSensor;
    };

    struct Box2DContactAdapter {
        enum class ShapeType{
            A, B
        };


        Box2DContactAdapter(b2Contact* contact): m_contact{contact} { }
        Box2DContactAdapter(const Box2DContactAdapter& other) = delete;
        Box2DContactAdapter& operator=(const Box2DContactAdapter& other) = delete;
        Box2DContactAdapter(Box2DContactAdapter&& other) = delete;
        Box2DContactAdapter& operator=(Box2DContactAdapter&& other) = delete;
        ~Box2DContactAdapter() = default;

        void setEnabled(bool flag) {
            m_contact -> SetEnabled(flag);
        }



        bool getShapeData(ContactShapeData& contactShapeData, ShapeType type) {
            SceneEntity* entity = nullptr;
            if(type == ShapeType::A)
                entity = getContactShapeA();
            if(type == ShapeType::B)
                entity = getContactShapeB();
            if(!entity)
                return false;

            contactShapeData.entityUuid = entity->getUUID();
            contactShapeData.fixtureUuid;

            const auto& c2d =  entity -> getComponent<Collider2DComponent>();

            contactShapeData.category = c2d.filter.categoryBits;
            contactShapeData.mask = c2d.filter.maskBits;
            contactShapeData.isSensor = c2d.isTrigger;

            return true;
        }
    private:
        SceneEntity* getContactShapeA() {
            m_shapeA = (SceneEntity*)(m_contact -> GetFixtureA() -> GetUserData().getData());
            return m_shapeA;
        }

        SceneEntity* getContactShapeB()  {
            m_shapeB = (SceneEntity*)(m_contact -> GetFixtureB() -> GetUserData().getData());
            return m_shapeB;
        }
    private:
        b2Contact* m_contact{nullptr};
        SceneEntity* m_shapeA { nullptr };
        SceneEntity* m_shapeB { nullptr };
    };

    using ContactCallback = std::function<void(const Box2DContactAdapter&)>;
    
    void Box2DPhysicsAdapter::BeginContact([[maybe_unused]] b2Contact* contact) {
        Box2DContactAdapter adapter(contact);
        ContactShapeData contactShapeDataA;
        ContactShapeData contactShapeDataB;
        if(!adapter.getShapeData(contactShapeDataA, Box2DContactAdapter::ShapeType::A)) {
            RB_EDITOR_WARN("Box2DPhysicsAdapter::Error GetShapeDataA");
            return;
        }

        if(!adapter.getShapeData(contactShapeDataB, Box2DContactAdapter::ShapeType::B)) {
            RB_EDITOR_WARN("Box2DPhysicsAdapter::Error GetShapeDataB");
            return;
        }

        uint64_t A,B;
        uint64_t fA, fB;
        uint16_t catA, catB, maskA, maskB;
        bool senA, senB;

        A = contactShapeDataA.entityUuid;
        B = contactShapeDataB.entityUuid;
        catA = contactShapeDataA.category;
        catB = contactShapeDataB.category;
        maskA = contactShapeDataA.mask;
        maskB = contactShapeDataB.mask;
        senA = contactShapeDataA.isSensor;
        senB = contactShapeDataB.isSensor;


        const bool trig = isTrigger(contact);
        PairKey key{A, B, trig};
        m_currentPairs.insert(key);

        Geom g;
        FillWorld(contact, g);
        m_geomCache[key] = g;

        PhysicsContact2D ev{};
        ev.entityA = A;
        ev.entityB = B;
        ev.fixtureA = fA;
        ev.fixtureB = fB;
        ev.type = trig ? PhysicsEventType::EnterTrigger : PhysicsEventType::Enter;
        ev.nx = g.nx;
        ev.ny = g.ny;
        RB_EDITOR_WARN("Geomerty: Normal {0}:{1}", g.nx, g.ny);
        RB_EDITOR_WARN("Offset ny: {0}", offsetof(PhysicsContact2D, ny));
        ev.pointCount = g.count;
        for (int i=0; i < g.count; i++)
            ev.points[i] = g.pts[i];
        ev.isSensorA = senA;
        ev.isSensorB = senB;
        ev.categoryA = catA;
        ev.categoryB = catB;
        ev.maskA = maskA;
        ev.maskB = maskB;
        m_eventsThisFrame.push_back(ev);
    }

    void Box2DPhysicsAdapter::EndContact([[maybe_unused]] b2Contact* contact) {
        Box2DContactAdapter adapter(contact);
        ContactShapeData contactShapeDataA;
        ContactShapeData contactShapeDataB;
        if(!adapter.getShapeData(contactShapeDataA, Box2DContactAdapter::ShapeType::A)) {
            RB_EDITOR_WARN("Box2DPhysicsAdapter::Error GetShapeDataA");
            return;
        }

        if(!adapter.getShapeData(contactShapeDataB, Box2DContactAdapter::ShapeType::B)) {
            RB_EDITOR_WARN("Box2DPhysicsAdapter::Error GetShapeDataB");
            return;
        }

        uint64_t A,B;
        uint64_t fA, fB;
        uint16_t catA, catB, maskA, maskB;
        bool senA, senB;

        A = contactShapeDataA.entityUuid;
        B = contactShapeDataB.entityUuid;
        catA = contactShapeDataA.category;
        catB = contactShapeDataB.category;
        maskA = contactShapeDataA.mask;
        maskB = contactShapeDataB.mask;
        senA = contactShapeDataA.isSensor;
        senB = contactShapeDataB.isSensor;

        const bool trig = isTrigger(contact);
        PhysicsContact2D ev{};
        ev.entityA=A; ev.entityB=B; ev.fixtureA=fA; ev.fixtureB=fB;
        ev.type = trig ? PhysicsEventType::ExitTrigger : PhysicsEventType::Exit;
        ev.isSensorA=senA; ev.isSensorB=senB; ev.categoryA=catA; ev.categoryB=catB; ev.maskA=maskA; ev.maskB=maskB;
        m_eventsThisFrame.push_back(ev);
    }



    void Box2DPhysicsAdapter::PreSolve([[maybe_unused]] b2Contact* contact, const b2Manifold* oldManifold) {
        Box2DContactAdapter adapter(contact);
        ContactShapeData contactShapeDataA;
        ContactShapeData contactShapeDataB;
        if(!adapter.getShapeData(contactShapeDataA, Box2DContactAdapter::ShapeType::A)) {
            RB_EDITOR_WARN("Box2DPhysicsAdapter::Error GetShapeDataA");
            return;
        }

        if(!adapter.getShapeData(contactShapeDataB, Box2DContactAdapter::ShapeType::B)) {
            RB_EDITOR_WARN("Box2DPhysicsAdapter::Error GetShapeDataB");
            return;
        }

        uint64_t A,B;
        uint64_t fA, fB;
        uint16_t catA, catB, maskA, maskB;
        bool senA, senB;

        A = contactShapeDataA.entityUuid;
        B = contactShapeDataB.entityUuid;
        catA = contactShapeDataA.category;
        catB = contactShapeDataB.category;
        maskA = contactShapeDataA.mask;
        maskB = contactShapeDataB.mask;
        senA = contactShapeDataA.isSensor;
        senB = contactShapeDataB.isSensor;

        {
            /// check MycurrentTimer

//            if ((udA && udA->dropThroughUntil > now) || (udB && udB->dropThroughUntil > now)) {
//                c->SetEnabled(false);
//                return;
//            }
        }

        const bool trig = isTrigger(contact);
        PairKey key{A,B,trig}; //  PairKey key{A < B ? A : B, A < B ? B : A, trig};
        Geom g;
        FillWorld(contact, g);
        m_geomCache[key] = g;
    }

    void Box2DPhysicsAdapter::PostSolve([[maybe_unused]] b2Contact* contact,
                                        [[maybe_unused]] const b2ContactImpulse* impulse) {
        Box2DContactAdapter adapter(contact);
        ContactShapeData contactShapeDataA;
        ContactShapeData contactShapeDataB;
        if(!adapter.getShapeData(contactShapeDataA, Box2DContactAdapter::ShapeType::A)) {
            RB_EDITOR_WARN("Box2DPhysicsAdapter::Error GetShapeDataA");
            return;
        }

        if(!adapter.getShapeData(contactShapeDataB, Box2DContactAdapter::ShapeType::B)) {
            RB_EDITOR_WARN("Box2DPhysicsAdapter::Error GetShapeDataB");
            return;
        }

        uint64_t A,B;
        uint64_t fA, fB;
        uint16_t catA, catB, maskA, maskB;
        bool senA, senB;

        A = contactShapeDataA.entityUuid;
        B = contactShapeDataB.entityUuid;
        catA = contactShapeDataA.category;
        catB = contactShapeDataB.category;
        maskA = contactShapeDataA.mask;
        maskB = contactShapeDataB.mask;
        senA = contactShapeDataA.isSensor;
        senB = contactShapeDataB.isSensor;

        const bool trig = isTrigger(contact);
        PairKey key{ A, B,trig};

        float n=0.f, t=0.f;
        for (int i=0; i < impulse ->count;i++) {
            n += impulse->normalImpulses[i];
            t += impulse->tangentImpulses[i];
        }
        m_impulses[key] = {n,t};
    }

    void Box2DPhysicsAdapter::update(float dt) {
        if(!m_physicsWorld) {
            RB_EDITOR_ERROR("Can't create Physics2D World inside Box2DPhysicsAdapter");
            //TODO(a.raag) make exception
        }

        m_eventsThisFrame.clear();

        m_physicsWorld -> Step(dt, velocityUpdateIterations,
                               positionUpdateIterations);

        for(auto& entity: m_entityList)
            updateEntity(entity);

        emitStayAndSwap();

        /// \brief send events to both sides
        for(const auto& ev: m_eventsThisFrame) {
            m_callback(ev, ev.entityA, ev.entityB);
            m_callback(ev, ev.entityB, ev.entityA);
        }
    }

    void Box2DPhysicsAdapter::updateEntity(SceneEntity entity) {
        if(!entity)
            return;

        auto& transform = entity.getComponent<TransformComponent>();
        if(entity.hasComponent<Physics2DComponent>()) {
            auto& p2c = entity.getComponent<Physics2DComponent>();

            auto* body = static_cast<b2Body*>(p2c.runtimeBody);
            const auto& position = body -> GetPosition();
            const float& angle = body -> GetAngle();

            transform.setPosition({meters_to_pixels(position.x), meters_to_pixels(position.y) });
            transform.rotate(angle);
        }
    }

    void Box2DPhysicsAdapter::start(editorEntityList& entityList) {
        m_physicsWorld = std::make_unique<b2World>(b2Vec2{0.f, 9.8f});
        if(!m_physicsWorld) {
            RB_EDITOR_ERROR("Can't create Physics2D World inside Box2DPhysicsAdapter");
            //TODO(a.raag) make exception
        }

        m_physicsWorld -> SetContactListener(this);

        for(auto& item: entityList)
            addEntity(item);

    }

    void Box2DPhysicsAdapter::addEntity(SceneEntity entity) {
        if(!entity)
            return;

        auto& transform = entity.getComponent<TransformComponent>();
        if(entity.hasComponent<Physics2DComponent>()) {
            auto& lEntity = m_entityList.emplace_back(entity);
            auto& rb2d = lEntity.getComponent<Physics2DComponent>();

            b2BodyDef b2BodyDef;
            switch(rb2d.type) {
                case Physics2DComponent::BodyType::Static:
                    b2BodyDef.type = b2BodyType::b2_staticBody;
                    break;
                case Physics2DComponent::BodyType::Dynamic:
                    b2BodyDef.type = b2BodyType::b2_dynamicBody;
                    break;
                case Physics2DComponent::BodyType::Kinematic:
                    b2BodyDef.type = b2BodyType::b2_kinematicBody;
                    break;
            }


            b2BodyDef.position.Set(pixel_to_meters<float>(transform.getPosition().x),
                                   pixel_to_meters<float>(transform.getPosition().y));
            b2BodyDef.angle = 0; // TODO(a.raag) rotation angle in rodians
            b2BodyDef.userData; // userData на тело — можно положить entityID

            auto* b2body = m_physicsWorld -> CreateBody(&b2BodyDef);
            b2body -> SetFixedRotation(rb2d.fixedRotation);
            b2body -> SetBullet(rb2d.bullet);

            b2body -> SetGravityScale(rb2d.gravityScale);
            b2body -> SetLinearDamping(rb2d.linearDamping);
            b2body -> SetAngularDamping(rb2d.angularDamping);

            rb2d.runtimeBody = b2body;


            if(lEntity.hasComponent<Collider2DComponent>()) {
                auto& c2d = lEntity.getComponent<Collider2DComponent>();
                b2PolygonShape polygonShape;

                auto center  = robot2D::vec2f {
                        pixel_to_meters<float>(transform.getSize().x) / 2.F,
                        pixel_to_meters<float>(transform.getSize().y) / 2.F
                };

                /// TODO(a.raag): realtime colliders view
                polygonShape.SetAsBox(center.x, center.y, {center.x, center.y}, 0);
                //polygonShape.SetAsBox(center.x, center.y);

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &polygonShape;

                fixtureDef.isSensor = c2d.isTrigger;
                const auto& fb = c2d.filter;
                fixtureDef.filter.categoryBits = fb.categoryBits;
                fixtureDef.filter.maskBits     = c2d.useDefaultMask
                                         ? phys2d::LayerRegistry::I().defaultMaskFor(
                                                 phys2d::layerNameFromCategoryBit(fb.categoryBits) )
                                         : fb.maskBits;
                fixtureDef.filter.groupIndex   = fb.groupIndex;

                if(b2BodyDef.type !=  b2BodyType::b2_kinematicBody) {
                    fixtureDef.density = c2d.density;
                    fixtureDef.friction = c2d.friction;
                    fixtureDef.restitution = c2d.restitution;
                    fixtureDef.restitutionThreshold = c2d.restitutionThreshold;
                }

                c2d.userData = {
                    .entityId = entity.getUUID(),
                    .oneWay   = c2d.oneWay,
                    .oneWayNormal = c2d.oneWayNormal, // в мировых или локальных? лучше в МИРОВЫХ
                    .generateEvents = true
                };

                fixtureDef.userData.setData((void*)&lEntity);
                auto fixture = b2body -> CreateFixture(&fixtureDef);

                c2d.runtimeFixture = fixture;
            }
        }

        for(auto& child: transform.getChildren())
            addEntity(child);
    }

    void Box2DPhysicsAdapter::stop() {
        m_entityList.clear();
        m_physicsWorld.reset();
    }

    void Box2DPhysicsAdapter::addRuntime(SceneEntity entity) {
        addEntity(entity);
    }

    bool Box2DPhysicsAdapter::isTrigger(const b2Contact* contact) {
        return contact -> GetFixtureA() -> IsSensor() || contact -> GetFixtureB() -> IsSensor();
    }

    void Box2DPhysicsAdapter::emitStayAndSwap() {
        for (auto& key : m_currentPairs) {
            if (m_prevPairs.find(key) != m_prevPairs.end()) {
                PhysicsContact2D ev{};
                ev.entityA = key.a; ev.entityB = key.b;
                ev.type = key.trigger ? PhysicsEventType::StayTrigger : PhysicsEventType::Stay;

                if (auto it = m_geomCache.find(key); it != m_geomCache.end()) {
                    ev.nx = it->second.nx; ev.ny = it->second.ny;
                    ev.pointCount = it->second.count;
                    for (int i=0;i<ev.pointCount;i++) ev.points[i] = it->second.pts[i];
                }
                if (auto it = m_impulses.find(key); it != m_impulses.end()) {
                    ev.normalImpulse  = it->second.first;
                    ev.tangentImpulse = it->second.second;
                }

                // category/mask/sensor для Stay — можно вытянуть из кэша фикстур (если хранишь) или опустить

                m_eventsThisFrame.push_back(ev);
            }
        }

        m_prevPairs.swap(m_currentPairs);
        m_currentPairs.clear();
        m_geomCache.clear();
        m_impulses.clear();
    }

    void Box2DPhysicsAdapter::FillWorld(b2Contact* c, editor::Box2DPhysicsAdapter::Geom& g) {
        b2WorldManifold wm;
        c -> GetWorldManifold(&wm);
        g.nx = wm.normal.x;
        g.ny = wm.normal.y;
        g.count = (uint8_t)c -> GetManifold() -> pointCount;
        for (int i=0;i<g.count;i++) {
            g.pts[i] = { wm.points[i].x, wm.points[i].y, wm.separations[i] };
        }
    }
}