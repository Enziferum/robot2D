#include "TestSupport.hpp"
#include <editor/Scene.hpp>
#include <editor/physics/Box2DPhysicsAdapter.hpp>
#include <editor/serializers/SceneSerializer.hpp>
#include <editor/scripting/ScriptingEngine.hpp>
#include <robot2D/Util/Logger.hpp>
#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>

namespace {
    using namespace editor;
    using namespace tests;
    constexpr float step = 1.f / 120.f;
    static_assert(offsetof(PhysicsContact2D, nx) == 33);
    static_assert(offsetof(PhysicsContact2D, pointCount) == 65);
    static_assert(offsetof(PhysicsContact2D, isSensorA) == 74);
    static_assert(offsetof(PhysicsContact2D, maskB) == 82);
    bool approximately(float a, float b, float tolerance = 0.01f) {
        return std::abs(a - b) <= tolerance;
    }
    b2Body* body(SceneEntity e) {
        return static_cast<b2Body*>(e.getComponent<Physics2DComponent>().runtimeBody);
    }
    b2Fixture* fixture(SceneEntity e) {
        return static_cast<b2Fixture*>(e.getComponent<Collider2DComponent>().runtimeFixture);
    }
    struct Event { PhysicsContact2D contact; editor::UUID self; editor::UUID other; };
    struct Fixture {
        robot2D::MessageBus bus;
        Scene scene{bus};
        Box2DPhysicsAdapter adapter;
        std::vector<Event> events;
        SceneEntity box(const std::string& name, robot2D::vec2f position, robot2D::vec2f size, bool dynamic) {
            auto e = scene.createEntity(name);
            e.getComponent<TransformComponent>().setPosition(position);
            e.getComponent<TransformComponent>().setSize(size);
            auto& p = e.addComponent<Physics2DComponent>();
            p.type = dynamic ? Physics2DComponent::BodyType::Dynamic : Physics2DComponent::BodyType::Static;
            p.gravityScale = 0.f;
            e.addComponent<Collider2DComponent>();
            return e;
        }
        void start() {
            auto roots = scene.getRootEntities(); adapter.start(roots);
            adapter.registerCallback([this](const auto& c, editor::UUID self, editor::UUID other) {
                events.push_back({c, self, other});
            });
        }
        int count(PhysicsEventType type, editor::UUID self) const {
            return static_cast<int>(std::count_if(events.begin(), events.end(), [&](const auto& e) {
                return e.self == self && e.contact.type == type;
            }));
        }
        void ticks(int n) {
            for(int i = 0; i < n; ++i) adapter.update(step);
        }
    };
    void geometry() {
        Fixture f;
        auto e = f.box("Box", {64, 96}, {32, 64}, true);
        auto& t = e.getComponent<TransformComponent>(); t.setOrigin({0.5f, 0.5f}); t.setRotate(90.f);
        auto& c = e.getComponent<Collider2DComponent>(); c.size = {0.25f, 0.125f}; c.offset = {8, -4};
        c.friction = 0.3f;
        f.start(); CHECK(body(e) && fixture(e));
        const auto* shape = static_cast<const b2PolygonShape*>(fixture(e) -> GetShape());
        CHECK(approximately(shape -> m_centroid.x, 0.25f) && approximately(shape -> m_centroid.y, -0.125f));
        float minX = 100.f, maxX = -100.f, minY = 100.f, maxY = -100.f;
        for(int i = 0; i < shape -> m_count; ++i) {
            minX = std::min(minX, shape -> m_vertices[i].x); maxX = std::max(maxX, shape -> m_vertices[i].x);
            minY = std::min(minY, shape -> m_vertices[i].y); maxY = std::max(maxY, shape -> m_vertices[i].y);
        }
        CHECK(approximately(maxX - minX, 0.5f) && approximately(maxY - minY, 0.5f));
        CHECK(approximately(fixture(e) -> GetFriction(), 0.3f));
        f.ticks(20); CHECK(approximately(t.getRotation(), 90.f));
        f.adapter.registerCallback({}); f.adapter.update(step);
    }
    void walls() {
        Fixture f;
        auto wall = f.box("Wall", {64, 0}, {32, 128}, false);
        auto player = f.box("Player", {0, 24}, {32, 32}, true);
        wall.getComponent<Collider2DComponent>().filter = phys2d::makeFilterDefault("Wall");
        player.getComponent<Collider2DComponent>().filter = phys2d::makeFilterDefault("Player");
        f.start(); body(player) -> SetLinearVelocity({4, 0}); f.ticks(240);
        CHECK(player.getComponent<TransformComponent>().getPosition().x < 33.f);
        CHECK(f.count(PhysicsEventType::Enter, player.getUUID()) == 1);
        CHECK(f.count(PhysicsEventType::Enter, wall.getUUID()) == 1);
        CHECK(f.count(PhysicsEventType::Stay, player.getUUID()) > 3);
        const auto event = f.events.front().contact;
        CHECK(event.fixtureA && event.fixtureB && event.fixtureA != event.fixtureB);
        CHECK(event.entityA < event.entityB && event.pointCount > 0 && event.pointCount <= 2);
        CHECK(event.categoryA && event.categoryB && event.maskA && event.maskB);
        body(player) -> SetLinearVelocity({0, 0}); body(player) -> SetAwake(false);
        const auto stays = f.count(PhysicsEventType::Stay, player.getUUID()); f.ticks(5);
        CHECK(f.count(PhysicsEventType::Stay, player.getUUID()) == stays + 5);
        body(player) -> SetTransform({-4, 0}, 0); body(player) -> SetAwake(true); f.ticks(3);
        CHECK(f.count(PhysicsEventType::Exit, player.getUUID()) == 1);
        CHECK(f.count(PhysicsEventType::Exit, wall.getUUID()) == 1);
    }
    void bullet() {
        Fixture f;
        auto wall = f.box("Wall", {200, 0}, {2, 128}, false);
        auto bullet = f.box("Projectile", {0, 40}, {8, 8}, true);
        wall.getComponent<Collider2DComponent>().filter = phys2d::makeFilterDefault("Wall");
        bullet.getComponent<Collider2DComponent>().filter = phys2d::makeFilterDefault("Projectile");
        bullet.getComponent<Physics2DComponent>().bullet = true;
        f.start(); body(bullet) -> SetLinearVelocity({200, 0}); f.ticks(120);
        CHECK(bullet.getComponent<TransformComponent>().getPosition().x <= 193.f);
        CHECK(f.count(PhysicsEventType::Enter, bullet.getUUID()) == 1);
        Fixture enemies;
        auto obstacle = enemies.box("Wall", {64, 0}, {32, 128}, false);
        auto enemy = enemies.box("Enemy", {0, 24}, {32, 32}, true);
        obstacle.getComponent<Collider2DComponent>().filter = phys2d::makeFilterDefault("Wall");
        enemy.getComponent<Collider2DComponent>().filter = phys2d::makeFilterDefault("Enemy");
        enemies.start(); body(enemy) -> SetLinearVelocity({4, 0}); enemies.ticks(120);
        CHECK(enemy.getComponent<TransformComponent>().getPosition().x < 33.f);
        CHECK(enemies.count(PhysicsEventType::Enter, enemy.getUUID()) == 1);
    }

    void triggers() {
        Fixture f;
        auto trigger = f.box("Trigger", {0, 0}, {64, 64}, false);
        auto player = f.box("Player", {8, 8}, {16, 16}, true);
        trigger.getComponent<Collider2DComponent>().isTrigger = true;
        trigger.getComponent<Collider2DComponent>().filter = phys2d::makeFilterDefault("Trigger");
        player.getComponent<Collider2DComponent>().filter = phys2d::makeFilterDefault("Player");
        f.start(); f.ticks(5);
        CHECK(f.count(PhysicsEventType::EnterTrigger, player.getUUID()) == 1);
        CHECK(f.count(PhysicsEventType::StayTrigger, player.getUUID()) == 4);
        CHECK(f.count(PhysicsEventType::Enter, player.getUUID()) == 0);
        for(const auto& e: f.events) CHECK(e.contact.pointCount == 0 && (e.contact.isSensorA || e.contact.isSensorB));
        body(player) -> SetTransform({4, 0}, 0); f.ticks(2);
        CHECK(f.count(PhysicsEventType::ExitTrigger, player.getUUID()) == 1);
        body(player) -> SetTransform({0.5f, 0.5f}, 0); f.ticks(2);
        const auto exits = f.count(PhysicsEventType::ExitTrigger, trigger.getUUID());
        f.adapter.removeRuntime(player); CHECK(!body(player) && !fixture(player));
        f.ticks(2); CHECK(f.count(PhysicsEventType::ExitTrigger, trigger.getUUID()) == exits + 1);
    }
    void filters() {
        Fixture f;
        auto a = f.box("Player", {0, 0}, {32, 32}, false);
        auto b = f.box("Projectile", {8, 8}, {16, 16}, true);
        auto& ca = a.getComponent<Collider2DComponent>(); auto& cb = b.getComponent<Collider2DComponent>();
        ca.isTrigger = true; ca.filter = phys2d::makeFilterDefault("Player");
        cb.filter = phys2d::makeFilterDefault("Projectile");
        f.start(); f.ticks(3); CHECK(f.events.empty());
        ca.useDefaultMask = cb.useDefaultMask = false;
        ca.filter = phys2d::makeFilterCustomMask("Player", {"Projectile"});
        cb.filter = phys2d::makeFilterCustomMask("Projectile", {"Player"});
        CHECK(ca.filter.name == "Player" && cb.filter.name == "Projectile");
        ca.markFilterDirty = cb.markFilterDirty = true; f.ticks(3);
        CHECK(f.count(PhysicsEventType::EnterTrigger, a.getUUID()) == 1);
        CHECK(f.events.front().contact.categoryA && f.events.front().contact.categoryB);
    }
    void lifecycle() {
        Fixture f;
        auto a = f.box("A", {0, 0}, {32, 32}, true);
        auto child = f.box("Child", {64, 0}, {16, 16}, true); CHECK(f.scene.reparent(child, a));
        f.start(); CHECK(body(a) -> GetWorld() -> GetBodyCount() == 2);
        f.adapter.addRuntime(a); CHECK(body(a) -> GetWorld() -> GetBodyCount() == 2);
        f.adapter.stop(); CHECK(!body(a) && !fixture(a) && !body(child) && !fixture(child));
        f.start(); CHECK(body(a) -> GetWorld() -> GetBodyCount() == 2);
        f.adapter.removeRuntime(a); CHECK(!body(a) && !body(child));
        f.adapter.removeRuntime(a); f.adapter.stop(); f.adapter.stop(); f.adapter.update(step);
        { Box2DPhysicsAdapter scoped; auto roots = f.scene.getRootEntities(); scoped.start(roots); CHECK(body(a)); }
        CHECK(!body(a) && !fixture(a));
    }
    void callbacks() {
        Fixture f;
        auto a = f.box("A", {0, 0}, {32, 32}, false);
        auto b = f.box("B", {8, 8}, {16, 16}, true);
        auto extra = f.scene.createEntity("Added"); extra.getComponent<TransformComponent>().setPosition({300, 0});
        f.start(); extra.addComponent<Physics2DComponent>(); int calls = 0; SceneEntity removed;
        f.adapter.registerCallback([&](const auto&, editor::UUID self, editor::UUID) {
            ++calls; removed = f.scene.getEntity(self);
            CHECK(!body(removed) -> GetWorld() -> IsLocked());
            f.adapter.addRuntime(extra); CHECK(!body(extra));
            f.adapter.removeRuntime(removed); CHECK(body(removed));
            f.adapter.update(step); // no reentrant Step or dispatch
        });
        f.adapter.update(step); CHECK(calls == 1 && !body(removed) && body(extra));
        f.adapter.registerCallback({}); f.adapter.update(step);
        f.adapter.stop(); CHECK(!body(extra));
        f.start(); calls = 0;
        f.adapter.registerCallback([&](const auto&, editor::UUID, editor::UUID) { ++calls; f.adapter.stop(); });
        f.adapter.update(step); CHECK(calls == 1 && !body(a) && !body(b));
    }
    void invalid() {
        Fixture f;
        auto bad = f.box("Bad", {0, 0}, {32, 32}, true);
        bad.getComponent<Collider2DComponent>().size = {0, 0}; f.start(); CHECK(!body(bad));
        bad.getComponent<Collider2DComponent>().size = {0.5f, 0.5f}; f.adapter.addRuntime(bad); CHECK(body(bad));
        const auto pos = body(bad) -> GetPosition();
        for(float dt: {0.f, -1.f, std::numeric_limits<float>::infinity(),
                       std::numeric_limits<float>::quiet_NaN()}) f.adapter.update(dt);
        CHECK(body(bad) -> GetPosition() == pos);
    }
    void serialization() {
        Fixture f; Directory files;
        auto a = f.box("Saved", {0, 0}, {32, 32}, true);
        auto& c = a.getComponent<Collider2DComponent>(); auto& p = a.getComponent<Physics2DComponent>();
        c.isTrigger = true; c.useDefaultMask = false;
        c.filter = phys2d::makeFilterCustomMask("Projectile", {"Enemy"}, -2);
        p.bullet = true; p.gravityScale = 0.f; p.linearDamping = 0.25f; p.angularDamping = 0.5f;
        const auto path = files / "physics.scene"; auto scene = std::shared_ptr<Scene>(&f.scene, [](Scene*) {});
        SceneSerializer writer(scene); CHECK(writer.serialize(path.u8string(), "Physics", {}));
        auto restored = std::make_shared<Scene>(f.bus); SceneSerializer reader(restored);
        CHECK(reader.deserialize(path.u8string(), {}));
        const auto entity = restored -> getEntity(a.getUUID());
        const auto& rc = entity.getComponent<Collider2DComponent>(); const auto& rp = entity.getComponent<Physics2DComponent>();
        CHECK(rc.isTrigger && !rc.useDefaultMask && rc.filter.name == "Projectile");
        CHECK(rc.filter.maskBits == c.filter.maskBits && rc.filter.groupIndex == -2);
        CHECK(rp.bullet && rp.gravityScale == 0.f && approximately(rp.linearDamping,
            0.25f) && approximately(rp.angularDamping, 0.5f));
    }
    void managed() {
        Fixture f;
        auto a = f.box("A", {0, 0}, {32, 32}, false); auto b = f.box("B", {24, 8}, {16, 16}, true);
        a.addComponent<ScriptComponent>().name = "PhysicsReceiver";
        b.addComponent<ScriptComponent>().name = "PhysicsReceiver";
        ScriptEngine engine;
        auto core = fs::path(RB2D_TEST_SCRIPTCORE); core.replace_extension(); engine.Init(core.u8string());
        engine.runtimeInit(RB2D_TEST_PHYSICS_DRIVER);
        auto scripts = std::make_shared<ScriptInteractor>(EditorInteractor::WeakPtr{}, &engine);
        engine.onRuntimeStart(scripts); engine.onCreateEntity(a); engine.onCreateEntity(b);
        auto ia = engine.getEntityScriptInstance(a.getUUID()), ib = engine.getEntityScriptInstance(b.getUUID());
        CHECK(ia && ib && ia -> getClassWrapper() -> getInstance() != ib -> getClassWrapper() -> getInstance());
        auto* assembly = mono_domain_assembly_open(mono_domain_get(), RB2D_TEST_PHYSICS_DRIVER); CHECK(assembly);
        auto* klass = mono_class_from_name(mono_assembly_get_image(assembly), "", "PhysicsDriver"); CHECK(klass);
        auto* method = mono_class_get_method_from_name(klass, "Layout", 0); CHECK(method);
        MonoObject* exception = nullptr; auto* value = mono_runtime_invoke(method, nullptr, nullptr, &exception);
        CHECK(!exception && value && *static_cast<int*>(mono_object_unbox(value)) == 1);
        f.start(); f.adapter.registerCallback([&](const auto& c, editor::UUID self, editor::UUID other) {
            engine.onPhysicsCallback(c, self, other);
        });
        f.ticks(3);
        const auto field = [](const ScriptInstance::Ptr& instance, const char* name) {
            int value = -1; CHECK(instance -> getFieldValueInternal(name, &value)); return value;
        };
        CHECK(field(ia, "EnterCount") == 1 && field(ib, "EnterCount") == 1);
        CHECK(field(ia, "StayCount") == 2 && field(ib, "StayCount") == 2);
        CHECK(field(ia, "Errors") == 0 && field(ib, "Errors") == 0);
        float ax = 0, ay = 0, bx = 0, by = 0;
        CHECK(ia -> getFieldValueInternal("NormalX", &ax) && ia -> getFieldValueInternal("NormalY", &ay));
        CHECK(ib -> getFieldValueInternal("NormalX", &bx) && ib -> getFieldValueInternal("NormalY", &by));
        CHECK(approximately(ax, -bx) && approximately(ay, -by) && std::abs(ax) + std::abs(ay) > 0.9f);
        body(b) -> SetTransform({4, 0}, 0); f.ticks(2);
        CHECK(field(ia, "ExitCount") == 1 && field(ib, "ExitCount") == 1);
        f.adapter.stop();
        a.getComponent<Collider2DComponent>().isTrigger = true;
        b.getComponent<TransformComponent>().setPosition({24, 8});
        f.start(); f.adapter.registerCallback([&](const auto& c, editor::UUID self, editor::UUID other) {
            engine.onPhysicsCallback(c, self, other);
        });
        const auto checkTriggers = [&](int enters, int stays, int exits, int phase) {
            for(const auto& instance: {ia, ib}) {
                CHECK(field(instance, "TriggerEnterCount") == enters);
                CHECK(field(instance, "TriggerStayCount") == stays);
                CHECK(field(instance, "TriggerExitCount") == exits);
                CHECK(field(instance, "TriggerPhase") == phase);
                CHECK(field(instance, "Errors") == 0);
                CHECK(field(instance, "EnterCount") == 1);
                CHECK(field(instance, "StayCount") == 2);
                CHECK(field(instance, "ExitCount") == 1);
            }
        };
        f.ticks(1); checkTriggers(1, 0, 0, 1);
        f.ticks(2); checkTriggers(1, 2, 0, 1);
        body(b) -> SetTransform({4, 0}, 0);
        f.ticks(3); checkTriggers(1, 2, 1, 2);
        body(b) -> SetTransform({0.75f, 0.25f}, 0);
        f.ticks(1); checkTriggers(2, 2, 1, 1);
        f.ticks(2); checkTriggers(2, 4, 1, 1);
        body(b) -> SetTransform({4, 0}, 0);
        f.ticks(3); checkTriggers(2, 4, 2, 2);
        f.adapter.stop(); checkTriggers(2, 4, 2, 2);
        engine.onRuntimeStop();
        ia.reset(); ib.reset(); scripts.reset(); ScriptGlue::setService(nullptr); engine.Shutdown();
    }
}
int main(int argc, char** argv) {
    logger::Log::Init();
    try {
        CHECK(argc == 2); const std::string scenario = argv[1];
        if(scenario == "geometry") geometry(); else if(scenario == "walls") walls();
        else if(scenario == "triggers") triggers(); else if(scenario == "filters") filters();
        else if(scenario == "lifecycle") lifecycle(); else if(scenario == "callbacks") callbacks();
        else if(scenario == "invalid") invalid(); else if(scenario == "serialization") serialization();
        else if(scenario == "managed") managed(); else if(scenario == "bullet") bullet(); else CHECK(false);
        return 0;
    }
    catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
