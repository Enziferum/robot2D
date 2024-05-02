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
#include <editor/Components.hpp>

namespace editor {

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

    struct Box2DShapeAdapter{};

    struct Box2DContactAdapter {
        Box2DContactAdapter(b2Contact* contact): m_contact{contact} { }
        ~Box2DContactAdapter() = default;

        void setEnabled(bool flag) {
            m_contact -> SetEnabled(flag);
        }

        robot2D::ecs::Entity* getContactShapeA() const {
            return (robot2D::ecs::Entity*)(m_contact -> GetFixtureA() -> GetUserData().getData());
        }

        robot2D::ecs::Entity* getContactShapeB() const {
            return (robot2D::ecs::Entity*)(m_contact -> GetFixtureB() -> GetUserData().getData());
        }
    private:
        b2Contact* m_contact{nullptr};
    };

    using ContactCallback = std::function<void(const Box2DContactAdapter&)>;
    
    void Box2DPhysicsAdapter::BeginContact([[maybe_unused]] b2Contact* contact) {
        Box2DContactAdapter contactAdapter(contact);

        auto entityA = contactAdapter.getContactShapeA();
        auto entityB = contactAdapter.getContactShapeB();

        Physics2DContact physics2DContact {};
        physics2DContact.entityA = entityA -> getComponent<IDComponent>().ID;
        physics2DContact.entityB = entityB -> getComponent<IDComponent>().ID;
        physics2DContact.contanctType = PhysicsCallbackType::Enter;
        m_callbacks[PhysicsCallbackType::Enter](physics2DContact);
    }

    void Box2DPhysicsAdapter::EndContact([[maybe_unused]] b2Contact* contact) {
        Box2DContactAdapter contactAdapter(contact);
        auto entityA = contactAdapter.getContactShapeA();
        auto entityB = contactAdapter.getContactShapeB();

        Physics2DContact physics2DContact {};
        physics2DContact.entityA = entityA -> getComponent<IDComponent>().ID;
        physics2DContact.entityB = entityB -> getComponent<IDComponent>().ID;
        physics2DContact.contanctType = PhysicsCallbackType::Exit;
        m_callbacks[PhysicsCallbackType::Exit](physics2DContact);
    }



    void Box2DPhysicsAdapter::PreSolve([[maybe_unused]] b2Contact* contact, const b2Manifold* oldManifold) {
        Box2DContactAdapter contactAdapter(contact);

        auto entityA = contactAdapter.getContactShapeA();
        auto entityB = contactAdapter.getContactShapeB();

        Physics2DContact physics2DContact {};
        physics2DContact.entityA = entityA -> getComponent<IDComponent>().ID;
        physics2DContact.entityB = entityB -> getComponent<IDComponent>().ID;
        physics2DContact.contanctType = PhysicsCallbackType::EnterTrigger;
        m_callbacks[PhysicsCallbackType::EnterTrigger](physics2DContact);
    }

    void Box2DPhysicsAdapter::PostSolve([[maybe_unused]] b2Contact* contact,
                                        [[maybe_unused]] const b2ContactImpulse* impulse) {
        Box2DContactAdapter contactAdapter(contact);

        auto entityA = contactAdapter.getContactShapeA();
        auto entityB = contactAdapter.getContactShapeB();

        Physics2DContact physics2DContact {};
        physics2DContact.entityA = entityA -> getComponent<IDComponent>().ID;
        physics2DContact.entityB = entityB -> getComponent<IDComponent>().ID;
        physics2DContact.contanctType = PhysicsCallbackType::ExitTrigger;

        m_callbacks[PhysicsCallbackType::ExitTrigger](physics2DContact);
    }

    void Box2DPhysicsAdapter::update(float dt) {
        if(!m_physicsWorld) {
            RB_EDITOR_ERROR("Can't create Physics2D World inside Box2DPhysicsAdapter");
            //TODO(a.raag) make exception
        }
        m_physicsWorld -> Step(dt, velocityUpdateIterations, positionUpdateIterations);

        for(auto& entity: m_entityList)
            updateEntity(entity);
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
            b2BodyDef.angle = 0;
            b2BodyDef.userData;

            auto* b2body = m_physicsWorld -> CreateBody(&b2BodyDef);
            b2body -> SetFixedRotation(rb2d.fixedRotation);
            rb2d.runtimeBody = b2body;


            if(lEntity.hasComponent<Collider2DComponent>()) {
                auto& c2d = lEntity.getComponent<Collider2DComponent>();
                b2PolygonShape polygonShape;

                auto center  = robot2D::vec2f {
                        pixel_to_meters<float>(transform.getScale().x) / 2.F,
                        pixel_to_meters<float>(transform.getScale().y) / 2.F
                };

                polygonShape.SetAsBox(center.x, center.y, {center.x, center.y}, 0);

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &polygonShape;
                if(b2BodyDef.type !=  b2BodyType::b2_kinematicBody) {
                    fixtureDef.density = c2d.density;
                    fixtureDef.friction = c2d.friction;
                    fixtureDef.restitution = c2d.restitution;
                    fixtureDef.restitution = c2d.restitutionThreshold;
                }

                fixtureDef.userData.setData((void*)&lEntity);
                b2body -> CreateFixture(&fixtureDef);
            }
        }

        for(auto& child: transform.getChildren())
            addEntity(child);
    }

    void Box2DPhysicsAdapter::stop() {
        m_physicsWorld.reset();
    }

    void Box2DPhysicsAdapter::addRuntime(SceneEntity entity) {
        addEntity(entity);
    }

}