#include "TestSupport.hpp"
#include <editor/Scene.hpp>
#include <editor/serializers/SceneSerializer.hpp>
#include <editor/scripting/ScriptingEngine.hpp>
#include <robot2D/Util/Logger.hpp>
#include <mono/jit/jit.h>
#include <array>
#include <cmath>
#include <iostream>

namespace {
    using namespace editor;
    using namespace tests;
    constexpr float step = 1.f / 120.f;
    std::array<bool, 512> keys{};
    bool keyDown(std::uint16_t key) {
        return key < keys.size() && keys[key];
    }

    class SceneScripts final : public ScriptInteractor {
    public:
        SceneScripts(Scene& scene, ScriptEngine& engine) : ScriptInteractor({}, &engine), m_scene(scene) {
        }
        SceneEntity getEntity(std::uint64_t id) override {
            return m_scene.getEntity(editor::UUID{id});
        }
        SceneEntity getEntity(const std::string& name) override {
            return m_scene.getEntity(name);
        }
    private:
        Scene& m_scene;
    };

    struct Game {
        robot2D::MessageBus bus;
        std::shared_ptr<Scene> scene{std::make_shared<Scene>(bus)};
        Box2DPhysicsAdapter physics;
        ScriptEngine engine;
        std::shared_ptr<SceneScripts> scripts{std::make_shared<SceneScripts>(*scene, engine)};
        SceneEntity player;
        robot2D::vec2f spawn;
        std::uint64_t support{0};

        Game() {
            keys.fill(false);
            auto core = fs::path(RB2D_TEST_SCRIPTCORE);
            core.replace_extension();
            engine.Init(core.u8string());
            CHECK(engine.runtimeInit(RB2D_TEST_PLATFORMER_ASSEMBLY));
            engine.setScriptInteractor(scripts);
            engine.onRuntimeStart(scripts);
            // Only the physical keyboard is replaced. Component access, callbacks,
            // managed gameplay, scene parsing and Box2D all use production code.
            mono_add_internal_call("robot2D.InternalCalls::Input_IsKeyDown", reinterpret_cast<const void*>(keyDown));
            scene -> setAssociatedProjectPath(RB2D_TEST_PLATFORMER_PROJECT);
            SceneSerializer reader(scene);
            CHECK(reader.deserialize((fs::path(RB2D_TEST_PLATFORMER_PROJECT) /
                "assets/scenes/Platformer.scene").u8string(), scripts));
            player = scene -> getEntity("Player");
            CHECK(player);
            spawn = position();
            auto roots = scene -> getRootEntities();
            physics.start(roots);
            physics.registerCallback([this](const auto& contact, editor::UUID self, editor::UUID other) {
                engine.onPhysicsCallback(contact, self, other);
                if(self == player.getUUID()) {
                    const float normalY = contact.entityA == self ? -contact.ny : contact.ny;
                    if((contact.type == PhysicsEventType::Enter || contact.type == PhysicsEventType::Stay) &&
                    normalY < -0.5f)
                        support = other;
                    else if(contact.type == PhysicsEventType::Exit && support == static_cast<std::uint64_t>(other))
                        support = 0;
                }
            });
            engine.onCreateEntity(player);
            CHECK(engine.getEntityScriptInstance(player.getUUID()));
        }
        ~Game() {
            physics.stop();
            engine.onRuntimeStop();
            engine.setScriptInteractor(nullptr);
            scripts.reset();
            ScriptGlue::setService(nullptr);
            engine.Shutdown();
        }
        b2Body* body() {
            return static_cast<b2Body*>(player.getComponent<Physics2DComponent>().runtimeBody);
        }
        robot2D::vec2f position() {
            return player.getComponent<TransformComponent>().getPosition();
        }
        void ticks(int count) {
            for(int i = 0; i < count; ++i) {
                engine.onUpdateEntity(player, step);
                physics.update(step);
            }
        }
        void settle() {
            ticks(120);
            CHECK(support == 10003 && std::abs(body() -> GetLinearVelocity().y) < 0.1f);
        }
    };

    void route(Game& game) {
        game.settle();
        const auto x = game.position().x;
        keys[65] = keys[68] = true;
        game.ticks(10);
        CHECK(std::abs(game.position().x - x) < 0.1f);
        keys.fill(false);
        const float launch[]{330, 580, 835};
        const float landing[]{537.5f, 792.5f, 1097.5f};
        constexpr float visibleCenter = (292.f + 649.f * 0.5f) / 1254.f * 104.f;
        for(int stage = 0; stage < 3; ++stage) {
            keys[68] = true;
            for(int frame = 0; frame < 400 && game.position().x + visibleCenter < launch[stage]; ++frame)
                game.ticks(1);
            CHECK(game.position().x + visibleCenter >= launch[stage]);
            keys[32] = true;
            game.ticks(1);
            CHECK(game.body() -> GetLinearVelocity().y < -13.f);
            for(int frame = 0; frame < 300; ++frame) {
                keys[68] = game.position().x + visibleCenter < landing[stage];
                game.ticks(1);
                if(game.support == static_cast<std::uint64_t>(10004 + stage) &&
                    std::abs(game.body() -> GetLinearVelocity().y) < 0.1f)
                    break;
            }
            CHECK(game.support == static_cast<std::uint64_t>(10004 + stage));
            game.ticks(30); // Held jump must not launch again on landing.
            CHECK(std::abs(game.body() -> GetLinearVelocity().y) < 0.1f);
            keys.fill(false);
            game.ticks(1);
        }
    }

    void recovery(Game& game) {
        game.settle();
        for(int attempt = 0; attempt < 3; ++attempt) {
            keys[65] = true;
            bool recovering = false;
            for(int frame = 0; frame < 500; ++frame) {
                game.ticks(1);
                if(game.body() -> GetType() == b2_kinematicBody) {
                    recovering = true;
                    break;
                }
            }
            CHECK(recovering);
            keys.fill(false);
            keys[32] = true;
            for(int frame = 0; frame < 300 && game.body() -> GetType() != b2_dynamicBody; ++frame)
                game.ticks(1);
            CHECK(game.body() -> GetType() == b2_dynamicBody);
            CHECK(std::abs(game.position().x - game.spawn.x) <= 1.6f);
            game.ticks(120);
            CHECK(game.support == 10003 && std::abs(game.position().y - game.spawn.y) < 5.f);
            CHECK(std::abs(game.body() -> GetLinearVelocity().y) < 0.1f);
            keys.fill(false);
            game.ticks(1);
        }
        keys[68] = true;
        game.ticks(30);
        keys.fill(false);
        keys[82] = true;
        game.ticks(180);
        CHECK(game.body() -> GetType() == b2_dynamicBody && game.support == 10003);
        CHECK(std::abs(game.position().x - game.spawn.x) <= 1.6f);
    }

    void contacts(Game& game) {
        game.settle();
        // Walk off the physical edge. Jump in the air must preserve downward velocity.
        keys[65] = true;
        for(int frame = 0; frame < 200 && game.support != 0; ++frame)
            game.ticks(1);
        CHECK(game.support == 0);
        keys.fill(false);
        keys[32] = true;
        game.ticks(1);
        CHECK(game.body() -> GetLinearVelocity().y >= 0);
    }

    void boundaries(Game& game) {
        game.settle();
        const float start = game.position().x;
        const auto obstacle = [&](const char* name, robot2D::vec2f position, robot2D::vec2f size) {
            auto entity = game.scene -> createEntity(name);
            auto& transform = entity.getComponent<TransformComponent>();
            transform.setPosition(position);
            transform.setSize(size);
            entity.addComponent<Physics2DComponent>();
            entity.addComponent<Collider2DComponent>().friction = 0.f;
            game.physics.addRuntime(entity);
        };
        obstacle("Test wall", {start + 80.f, 450.f}, {16.f, 220.f});
        keys[68] = true;
        game.ticks(90);
        CHECK(game.position().x > start && game.position().x < start + 25.f);
        keys.fill(false);
        obstacle("Test ceiling", {start - 30.f, 500.f}, {180.f, 12.f});
        keys[32] = true;
        game.ticks(1);
        CHECK(game.body() -> GetLinearVelocity().y < -13.f);
        keys.fill(false);
        for(int frame = 0; frame < 30 && game.body() -> GetLinearVelocity().y < 0; ++frame)
            game.ticks(1);
        CHECK(game.body() -> GetLinearVelocity().y >= 0);
        keys[32] = true;
        game.ticks(1);
        CHECK(game.body() -> GetLinearVelocity().y >= 0); // Ceiling/wall cannot grant a new jump.
    }
}

int main(int argc, char** argv) {
    logger::Log::Init();
    try {
        CHECK(argc == 2);
        Game game;
        const std::string scenario = argv[1];
        if(scenario == "route") route(game);
        else if(scenario == "recovery") recovery(game);
        else if(scenario == "contacts") contacts(game);
        else if(scenario == "boundaries") boundaries(game);
        else CHECK(false);
        return 0;
    }
    catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
