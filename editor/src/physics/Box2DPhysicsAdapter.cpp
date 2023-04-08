#include <robot2D/Util/Logger.hpp>
#include <editor/physics/Box2DPhysicsAdapter.hpp>
#include <editor/Components.hpp>
#include <robot2D/Ecs/Entity.hpp>

namespace editor {

    namespace {
        constexpr int velocityUpdateIterations = 6;
        constexpr int positionUpdateIterations = 2;

        constexpr double PIXELS_PER_METERS = 32.0;
        constexpr double PI = 3.14159265358979323846;

        template<typename T>
        constexpr T pixel_to_meters(const T& x)
        {
            return x/PIXELS_PER_METERS;
        };

        template<typename T>
        constexpr T meters_to_pixels(const T& x)
        {
            return x*PIXELS_PER_METERS;
        }
    }

    struct Box2DShapeAdapter{};

    struct Box2DContactAdapter {
        Box2DContactAdapter(b2Contact* contact): m_contact{contact} { }
        ~Box2DContactAdapter() = default;

        void setEnabled(bool flag) { m_contact -> SetEnabled(flag); }

        robot2D::ecs::Entity* getContactShapeA() const {
            return static_cast<robot2D::ecs::Entity*>(m_contact -> GetFixtureA() -> GetUserData().getData());
        }

        robot2D::ecs::Entity* getContactShapeB() const {
            return static_cast<robot2D::ecs::Entity*>(m_contact -> GetFixtureB() -> GetUserData().getData());
        }
    private:
        b2Contact* m_contact{nullptr};
    };

    using ContactCallback = std::function<void(const Box2DContactAdapter&)>;

    void Box2DPhysicsAdapter::BeginContact([[maybe_unused]] b2Contact* contact) {
       // RB_EDITOR_INFO("Box2DPhysicsAdapter::BeginContact");
        Box2DContactAdapter contactAdapter(contact);
    }

    void Box2DPhysicsAdapter::EndContact([[maybe_unused]] b2Contact* contact) {
       // RB_EDITOR_INFO("Box2DPhysicsAdapter::EndContact");
    }



    void Box2DPhysicsAdapter::PreSolve([[maybe_unused]] b2Contact* contact, const b2Manifold* oldManifold) {
        //RB_EDITOR_INFO("Box2DPhysicsAdapter::PreSolve");
        Box2DContactAdapter contactAdapter(contact);

        auto entityA = contactAdapter.getContactShapeA();
        auto entityB = contactAdapter.getContactShapeB();

        Physics2DContact physics2DContact {};
        physics2DContact.entityA = entityA -> getIndex();
        physics2DContact.entityB = entityB -> getIndex();

        m_callbacks[PhysicsCallbackType::Enter](physics2DContact);
    }

    void Box2DPhysicsAdapter::PostSolve([[maybe_unused]] b2Contact* contact,
                                        [[maybe_unused]] const b2ContactImpulse* impulse) {
        //RB_EDITOR_INFO("Box2DPhysicsAdapter::PostSolve");
        Physics2DContact physics2DContact;
        m_callbacks[PhysicsCallbackType::Exit](physics2DContact);
    }

    void Box2DPhysicsAdapter::update(float dt) {
        if(!m_physicsWorld) {
            RB_EDITOR_ERROR("Can't create Physics2D World inside Box2DPhysicsAdapter");
            //TODO(a.raag) make exception
        }
        m_physicsWorld -> Step(dt, velocityUpdateIterations, positionUpdateIterations);

        for(auto& entity: *m_entityList) {
            if(!entity.hasComponent<Physics2DComponent>())
                continue;
            auto& transform = entity.getComponent<TransformComponent>();
            auto& p2c = entity.getComponent<Physics2DComponent>();

            auto* body = static_cast<b2Body*>(p2c.runtimeBody);
            const auto& position = body -> GetPosition();
            const float& angle = body -> GetAngle();

            transform.setPosition({meters_to_pixels(position.x), meters_to_pixels(position.y) });
            transform.setRotate(angle);
        }
    }

    void Box2DPhysicsAdapter::start(editorEntityList& entityList) {
        m_physicsWorld = std::make_unique<b2World>(b2Vec2{0.f, 9.8f});
        if(!m_physicsWorld) {
            RB_EDITOR_ERROR("Can't create Physics2D World inside Box2DPhysicsAdapter");
            //TODO(a.raag) make exception
        }

        m_physicsWorld -> SetContactListener(this);
        m_entityList = &entityList;

        for(auto& entity: *m_entityList) {
            if(!entity.hasComponent<Physics2DComponent>())
                continue;
            auto& transform = entity.getComponent<TransformComponent>();
            auto& rb2d = entity.getComponent<Physics2DComponent>();

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
            b2BodyDef.angle = 0;

            auto* b2body = m_physicsWorld -> CreateBody(&b2BodyDef);
            b2body -> SetFixedRotation(rb2d.fixedRotation);
            rb2d.runtimeBody = b2body;


            if(entity.hasComponent<Collider2DComponent>()) {
                auto& c2d = entity.getComponent<Collider2DComponent>();
                b2PolygonShape polygonShape;

                auto center  = robot2D::vec2f {
                        pixel_to_meters<float>(transform.getScale().x) / 2.F,
                        pixel_to_meters<float>(transform.getScale().y) / 2.F
                };

                polygonShape.SetAsBox(center.x, center.y, {center.x, center.y}, 0);

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &polygonShape;
                fixtureDef.density = c2d.density;
                fixtureDef.friction = c2d.friction;
                fixtureDef.restitution = c2d.restitution;
                fixtureDef.restitution = c2d.restitutionThreshold;
                fixtureDef.userData.setData(&entity);

                b2body -> CreateFixture(&fixtureDef);
            }
        }

    }

    void Box2DPhysicsAdapter::stop() {
        m_physicsWorld.reset();
    }
}