#include "TestSupport.hpp"
#include <editor/AnimationPlayback.hpp>
#include <editor/AnimationParser.hpp>
#include <editor/AnimatorSystem.hpp>
#include <editor/AnimationSystem.hpp>
#include <editor/Components.hpp>
#include <editor/LocalResourceManager.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/ScriptInteractor.hpp>
#include <editor/scripting/ScriptingEngineInternalService.hpp>
#include <editor/scripting/ScriptingGlue.hpp>
#include <robot2D/Ecs/Scene.hpp>
#include <robot2D/Util/Logger.hpp>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>
#include <array>
#include <iostream>
#include <limits>

namespace {
    using namespace editor;
    using namespace tests;

    Animation clip(const std::string& name, bool loop = false) {
        Animation result;
        result.name = name;
        result.texturePath = "assets/sheet.png";
        result.framesPerSecond = 4;
        result.isLooped = loop;
        for(int i = 0; i < 3; ++i)
            result.addFrame({i* 10, 0, 10, 10});
        return result;
    }

    struct Fixture {
        robot2D::MessageBus bus;
        robot2D::ecs::Scene scene{bus};
        SceneEntity entity;
        Fixture() {
            scene.addSystem<AnimatorSystem>(bus);
            auto raw = scene.createEntity();
            raw.addComponent<IDComponent>().ID = editor::UUID{7101};
            raw.addComponent<TransformComponent>();
            raw.addComponent<DrawableComponent>();
            raw.addComponent<AnimationComponent>();
            raw.addComponent<AnimatorComponent>();
            entity = SceneEntity{raw};
            scene.update(0.f);
        }
        AnimatorComponent& animator() {
            return entity.getComponent<AnimatorComponent>();
        }
        AnimationComponent& animation() {
            return entity.getComponent<AnimationComponent>();
        }
    };

    void frames() {
        Animation animation;
        CHECK(animation.sliceGrid({32, 16}, {16, 8}));
        CHECK(animation.frames.size() == 4);
        CHECK(animation.getFrame(0) -> lx == 0 && animation.getFrame(0) -> ly == 0);
        CHECK(animation.getFrame(1) -> lx == 16 && animation.getFrame(1) -> ly == 0);
        CHECK(animation.getFrame(2) -> lx == 0 && animation.getFrame(2) -> ly == 8);
        animation.setFrameIndices();
        CHECK(animation.getFrame(0) && !animation.getFrame(4));
        CHECK(!animation.sliceGrid({32, 16}, {0, 8}));
        CHECK(!animation.sliceGrid({32, 16}, {7, 8}));
        CHECK(!animation.sliceGrid({100000, 100000}, {1, 1}));
        CHECK(animation.frames.size() == 4);
        CHECK(animation.moveFrame(0, 3));
        CHECK(animation.getFrame(3) -> lx == 0 && animation.getFrame(3) -> ly == 0);
        CHECK(animation.moveFrame(3, 0));
        CHECK(!animation.moveFrame(0, 4));
        CHECK(!animation.removeFrame(-1));
        animation.isFlipped = true;
        CHECK(animation.getFrame(0) -> width == -16 && animation.getFrame(0) -> lx == 16);
        animation.isFlipped = false;
        std::vector<AnimationFrame> selected{animation.frames.front(), *std::next(animation.frames.begin(), 2)};
        AnimationFrame replacement;
        replacement.addRect({0, 0, 16, 16});
        animation.replaceFrames(selected, replacement);
        CHECK(animation.frames.size() == 3);
        CHECK(animation.getFrame(0) -> height == 16);
        CHECK(animation.getFrame(1) -> lx == 16);
        selected.clear();
        animation.replaceFrames(selected, replacement);
        CHECK(animation.frames.size() == 3);
        CHECK(animation.removeFrame(1));
        CHECK(animation.frames.size() == 2 && animation.getFrame(1));
    }

    void playback() {
        Fixture f;
        auto idle = clip("Idle");
        f.animation().setAnimation(&idle);
        CHECK(playAnimation(f.entity, "Idle"));
        CHECK(f.animation().getTextureRect().lx == 0);
        f.scene.update(0.375f);
        CHECK(f.animator().frameIndex() == 1);
        CHECK(playAnimation(f.entity, "Idle"));
        f.scene.update(0.125f);
        CHECK(f.animator().frameIndex() == 2);
        f.scene.update(0.75f);
        CHECK(!f.animator().playing());
        CHECK(f.animation().getTextureRect().lx == 20);
        CHECK(playAnimation(f.entity, "Idle"));
        CHECK(f.animation().getTextureRect().lx == 0);
        idle.isLooped = true;
        f.scene.update(1.375f);
        CHECK(f.animator().frameIndex() == 2);
        f.scene.update(0.125f);
        CHECK(f.animator().frameIndex() == 0);
        f.scene.update(-1.f);
        f.scene.update(std::numeric_limits<float>::quiet_NaN());
        CHECK(f.animator().frameIndex() == 0);
        f.scene.update(std::numeric_limits<float>::max());
        CHECK(f.animator().frameIndex() < 3);
        idle.frames.clear();
        f.scene.update(0.f);
        CHECK(!f.animator().playing());
        auto incomplete = f.scene.createEntity();
        incomplete.addComponent<TransformComponent>();
        incomplete.addComponent<DrawableComponent>();
        incomplete.addComponent<AnimatorComponent>().Play("Missing");
        f.scene.update(0.1f); // A missing AnimationComponent must not enter AnimatorSystem.
        CHECK(!playAnimation({}, "Idle"));
    }

    void runtimeClone() {
        Fixture f;
        f.scene.addSystem<AnimationSystem>(f.bus);
        auto idle = clip("Idle", true);
        f.animation().setAnimation(&idle);
        CHECK(playAnimation(f.entity, "Idle"));
        f.scene.update(0.375f);
        CHECK(f.animator().frameIndex() == 1);
        robot2D::ecs::Scene runtime{f.bus};
        std::vector<robot2D::ecs::Entity> copies;
        CHECK(f.scene.cloneSelf(runtime, copies, true));
        CHECK(runtime.getSystem<AnimatorSystem>());
        CHECK(runtime.getSystem<AnimationSystem>());
        CHECK(copies.size() == 1);
        SceneEntity copy{copies.front()};
        CHECK(!copy.getComponent<AnimatorComponent>().playing());
        CHECK(playAnimation(copy, "Idle"));
        runtime.update(0.5f);
        CHECK(copy.getComponent<AnimatorComponent>().frameIndex() == 2);
        CHECK(f.animator().frameIndex() == 1);
        CHECK(f.animation().getTextureRect().lx == 10);
        CHECK(copy.getComponent<AnimationComponent>().getTextureRect().lx == 20);
    }

    void clips() {
        Fixture f;
        auto* manager = LocalResourceManager::getManager();
        auto* idle = manager -> addAnimation(f.entity.getUUID());
        *idle = clip("Idle", true);
        f.animation().setAnimation(idle);
        for(int i = 0; i < 1024; ++i) {
            auto* added = manager -> addAnimation(f.entity.getUUID());
            *added = clip("Clip" + std::to_string(i));
        }
        CHECK(f.animation().getAnimation() == idle && idle -> name == "Idle");
        for(const auto& name: {"Move", "Attack", "Death"})
            *manager -> addAnimation(f.entity.getUUID()) = clip(name);
        for(const auto& name: {"Idle", "Move", "Attack", "Death"}) {
            CHECK(playAnimation(f.entity, name));
            CHECK(f.animator().animationName() == name);
        }
        CHECK(!playAnimation(f.entity, "Absent"));
        CHECK(f.animator().animationName() == "Death" && f.animator().playing());
        CHECK(!stopAnimation(f.entity, "Idle") && f.animator().playing());
        CHECK(stopAnimation(f.entity) && !f.animator().playing());
        auto* duplicate = manager -> addAnimation(f.entity.getUUID());
        *duplicate = clip("Idle");
        CHECK(!playAnimation(f.entity, "Idle"));
        auto* invalid = manager -> getAnimation(f.entity.getUUID(), "Death");
        invalid -> framesPerSecond = 0;
        CHECK(!playAnimation(f.entity, "Death"));
        auto* resources = ResourceManager::getManager();
        auto* first = resources -> addAnimation(editor::UUID{7102}, "First");
        for(int i = 0; i < 1024; ++i)
            resources -> addAnimation(editor::UUID{7102}, std::to_string(i));
        CHECK(first -> name == "First");
    }

    void parser() {
        Directory directory;
        const auto path = directory / "ordered clip.anim";
        auto animation = clip("Move", true);
        animation.frames.reverse();
        animation.setFrameIndices();
        AnimationParser parser;
        CHECK(parser.saveToFile(path.u8string(), &animation));
        Animation loaded;
        CHECK(parser.loadFromFile(path.u8string(), &loaded));
        CHECK(loaded.name == "Move" && loaded.isLooped && loaded.framesPerSecond == 4);
        CHECK(loaded.getFrame(0) -> lx == 20 && loaded.getFrame(2) -> lx == 0);
        CHECK(parser.loadFromFile(path.u8string(), &loaded));
        CHECK(loaded.frames.size() == 3);
        const auto saved = read(path);
        animation.framesPerSecond = 0;
        CHECK(!parser.saveToFile(path.u8string(), &animation));
        CHECK(read(path) == saved);
        const std::vector<std::string> invalid{
            "<sprites/>",
            "<sprites image='x'><animation delay='4'><cut x='0' y='0' w='1' h='1'/></animation></sprites>",
            "<sprites image='x'><animation title='X' delay='0'><cut x='0' y='0' w='1' h='1'/></animation></sprites>",
            "<sprites image='x'><animation title='X' delay='4'/></sprites>",
            "<sprites image='x'><animation title='X' delay='4'><cut y='0' w='1' h='1'/></animation></sprites>",
            "<sprites image='x'><animation title='X' delay='4'><cut x='0' y='0' w='0' h='1'/></animation></sprites>",
            "<sprites image='x'><animation title='X' delay='4' loop='bad'><cut x='0' y='0' w='1' h='1'/></animation></sprites>",
            "<sprites image='x'><animation/><animation/></sprites>"};
        for(const auto& xml: invalid) {
            std::ofstream(path) << xml;
            CHECK(!parser.loadFromFile(path.u8string(), &loaded));
            CHECK(loaded.name == "Move" && loaded.frames.size() == 3);
        }
        std::ofstream(path) <<
            "<sprites image='x'><animation title='Old' delay='4'><cut x='0' y='0' w='1' h='1'/></animation></sprites>";
        CHECK(parser.loadFromFile(path.u8string(), &loaded));
        CHECK(!loaded.isLooped && loaded.getFrame(0));
    }

    class Interactor final : public IScriptInteractor {
    public:
        SceneEntity entity;
        SceneEntity getEntity(std::uint64_t id) override {
            return id == entity.getUUID() ? entity : SceneEntity{};
        }
        SceneEntity getEntity(const std::string&) override {
            return {};
        }
        bool loadSceneRuntime(std::string&&) override {
            return false;
        }
        void loadSceneAsyncRuntime(std::string&&) override {
        }
        void exitEngineRuntime() override {
        }
        SceneEntity duplicateRuntime(SceneEntity, robot2D::vec2f) override {
            return {};
        }
    };

    class Service final : public ScriptingEngineInternalService {
    public:
        MonoImage* image{nullptr};
        void onCreateEntity(SceneEntity) override {
        }
        robot2D::Window* GetWindow() const override {
            return nullptr;
        }
        IEditorCamera::Ptr GetCamera() const override {
            return {};
        }
        MonoImage* GetCoreAssemblyImage() const override {
            return image;
        }
        MonoClassWrapper::Ptr getManagedObject(editor::UUID) const override {
            return {};
        }
    };

    void managed() {
        Fixture f;
        auto idle = clip("Idle", true);
        f.animation().setAnimation(&idle);
        mono_set_dirs(RB2D_TEST_MONO_LIB, RB2D_TEST_MONO_ETC);
        auto* domain = mono_jit_init("AnimationRegression");
        CHECK(domain);
        auto* assembly = mono_domain_assembly_open(domain, RB2D_TEST_SCRIPTCORE);
        CHECK(assembly);
        Service service;
        service.image = mono_assembly_get_image(assembly);
        auto interactor = std::make_shared<Interactor>();
        interactor -> entity = f.entity;
        service.setScriptInteractor(interactor);
        ScriptGlue::setService(&service);
        ScriptGlue::registerFunctions();
        ScriptGlue::registerComponents();
        auto* driver = mono_domain_assembly_open(domain, RB2D_TEST_ANIMATION_DRIVER);
        CHECK(driver);
        auto* klass = mono_class_from_name(mono_assembly_get_image(driver), "", "AnimationDriver");
        CHECK(klass);
        const auto invoke = [klass](const char* name, int count, void** args) {
            auto* method = mono_class_get_method_from_name(klass, name, count);
            CHECK(method);
            MonoObject* exception = nullptr;
            auto* result = mono_runtime_invoke(method, nullptr, args, &exception);
            CHECK(!exception && result);
            return *static_cast<std::int32_t*>(mono_object_unbox(result));
        };
        std::uint64_t id = f.entity.getUUID();
        void* playArgs[]{&id, mono_string_new(domain, "Idle")};
        void* idArgs[]{&id};
        CHECK(invoke("Play", 2, playArgs) == 1);
        f.scene.update(0.375f);
        CHECK(invoke("Play", 2, playArgs) == 1);
        f.scene.update(0.125f);
        CHECK(f.animator().frameIndex() == 2);
        playArgs[1] = mono_string_new(domain, "Absent");
        CHECK(invoke("Play", 2, playArgs) == 0 && f.animator().playing());
        playArgs[1] = nullptr;
        CHECK(invoke("Play", 2, playArgs) == 0);
        CHECK(invoke("Stop", 1, idArgs) == 0 && !f.animator().playing());
        playArgs[1] = mono_string_new(domain, "Idle");
        CHECK(invoke("LegacyPlay", 2, playArgs) == 1);
        CHECK(invoke("Playing", 1, idArgs) == 1);
        CHECK(invoke("Detached", 0, nullptr) == 0);
        id = 99999;
        CHECK(invoke("Probe", 1, idArgs) == 0);
        ScriptGlue::setService(nullptr);
        CHECK(invoke("Probe", 1, idArgs) == 0);
        mono_jit_cleanup(domain);
    }
}

int main(int argc, char** argv) {
    logger::Log::Init();
    try {
        CHECK(argc == 2);
        const std::string name = argv[1];
        if(name == "frames") frames();
        else if(name == "playback") playback();
        else if(name == "runtime-clone") runtimeClone();
        else if(name == "clips") clips();
        else if(name == "parser") parser();
        else if(name == "managed") managed();
        else CHECK(false);
        std::cout << name << " passed\n";
        return 0;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
