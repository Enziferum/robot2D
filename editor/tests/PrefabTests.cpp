#include "TestSupport.hpp"
#include <editor/PrefabManager.hpp>
#include <editor/components/ButtonComponent.hpp>
#include <editor/serializers/PrefabSerializer.hpp>
#include <editor/LocalResourceManager.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/FileApi.hpp>
#include <editor/AnimationParser.hpp>
#include <robot2D/Util/Logger.hpp>
#include <iostream>

namespace {
    using namespace editor;
    using namespace tests;

    class Scripts final : public IScriptInteractorFrom {
    public:
        std::unordered_map<editor::UUID, FieldMap> fields;
        bool hasEntityClass(const std::string&) const override {
            return true;
        }
        void setScriptClass(std::string, editor::UUID) override {
        }
        bool hasFields(editor::UUID id) const override {
            return fields.count(id) != 0;
        }
        FieldMap& getFields(editor::UUID id) override {
            return fields[id];
        }
        std::vector<std::string> getClassesNames() const override {
            return {"Enemy"};
        }
        void callScriptMethod(editor::UUID, const std::string&) override {
        }
        void setEditorCamera(EditorCamera::Ptr) override {
        }
        ScriptingEngineService* getScriptingEngine() const override {
            return nullptr;
        }
    };

    void reference(Scripts& scripts, editor::UUID owner, const std::string& name, FieldType type, editor::UUID target) {
        auto& field = scripts.fields[owner][name];
        field.setName(name);
        field.setType(type);
        field.setValue(target);
    }

    struct Fixture {
        Directory directory;
        robot2D::MessageBus bus;
        std::shared_ptr<Scripts> scripts = std::make_shared<Scripts>();
        Scene scene{bus};
        SceneEntity root;
        SceneEntity child;
        std::string path;
        Fixture() {
            fs::create_directories(directory / "assets/enemies");
            scene.setAssociatedProjectPath(directory.path().u8string());
            root = scene.createEntity("Enemy");
            child = scene.createEntity("Weapon");
            CHECK(scene.reparent(child, root));
            root.addComponent<ScriptComponent>().name = "Enemy";
            reference(*scripts, root.getUUID(), "Target", FieldType::Entity, child.getUUID());
            reference(*scripts, root.getUUID(), "Self", FieldType::Transform, root.getUUID());
            child.addComponent<ButtonComponent>().scriptEntity = root.getUUID();
            child.getComponent<ButtonComponent>().clickMethodName = "Fire";
            root.addComponent<Physics2DComponent>();
            child.addComponent<Collider2DComponent>();
            path = (directory / "assets/enemies/enemy.prefab").u8string();
        }
        void save() {
            auto prefab = std::make_shared<Prefab>(path, root, editor::UUID{9001});
            prefab -> setProjectPath(directory.path().u8string());
            PrefabSerializer serializer;
            CHECK(serializer.serialize(prefab, scripts));
        }
    };

    void instances() {
        Fixture f;
        f.save();
        auto library = std::make_unique<PrefabManager>();
        auto prefab = library -> loadPrefab(f.directory.path().u8string(), "assets/enemies/enemy.prefab", f.scripts);
        CHECK(prefab && prefab -> getTemplateScene());
        CHECK(library -> loadPrefab(f.directory.path().u8string(), f.path, f.scripts) == prefab);
        Scene target{f.bus};
        CHECK(target.getRootEntities().empty());
        auto first = target.instantiateTemplate(prefab -> getEntity(), f.scripts);
        auto second = target.instantiateTemplate(prefab -> getEntity(), f.scripts);
        CHECK(first && second && first.getUUID() != second.getUUID());
        CHECK(first.getUUID() != prefab -> getEntity().getUUID());
        CHECK(target.getRootEntities().size() == 2);
        const auto a = first.getChildren().front();
        const auto b = second.getChildren().front();
        CHECK(a != b && a.getParent() == first && b.getParent() == second);
        CHECK(a.getComponent<ButtonComponent>().scriptEntity == first.getUUID());
        CHECK(b.getComponent<ButtonComponent>().scriptEntity == second.getUUID());
        CHECK(!a.getComponent<ButtonComponent>().onClickCallback);
        CHECK(*f.scripts -> fields[first.getUUID()]["Target"].getValue<editor::UUID>() == a.getUUID());
        CHECK(*f.scripts -> fields[first.getUUID()]["Self"].getValue<editor::UUID>() == first.getUUID());
        CHECK(first.getComponent<PrefabComponent>().prefabUUID == prefab -> getUUID());
        first.getComponent<TransformComponent>().setPosition({12.f, 34.f});
        CHECK(second.getComponent<TransformComponent>().getPosition().x != 12.f);
        CHECK(prefab -> getEntity().getComponent<TransformComponent>().getPosition().x != 12.f);
        reference(*f.scripts, first.getUUID(), "Target", FieldType::Entity, editor::UUID{0});
        CHECK(*f.scripts -> fields[second.getUUID()]["Target"].getValue<editor::UUID>() == b.getUUID());
        CHECK(!first.getComponent<Physics2DComponent>().runtimeBody);
        CHECK(!a.getComponent<Collider2DComponent>().runtimeFixture);
        // The library can go away: copied components/fields/hierarchy belong to target.
        prefab.reset();
        library.reset();
        CHECK(first && a && a.getParent() == first);
    }

    void invalid() {
        Fixture f;
        f.save();
        const auto saved = read(fs::u8path(f.path));
        reference(*f.scripts, f.root.getUUID(), "Target", FieldType::Entity, editor::UUID{123});
        auto asset = std::make_shared<Prefab>(f.path, f.root, editor::UUID{9001});
        PrefabSerializer serializer;
        CHECK(!serializer.serialize(asset, f.scripts));
        CHECK(read(fs::u8path(f.path)) == saved);
        Scene target{f.bus};
        CHECK(!target.instantiateTemplate(f.root, f.scripts));
        CHECK(target.getRootEntities().empty());
        PrefabManager library;
        CHECK(!library.loadPrefab(f.directory.path().u8string(), "../outside.prefab", f.scripts));
        CHECK(!library.loadPrefab(f.directory.path().u8string(), "assets/missing.prefab", f.scripts));
        CHECK(!library.loadPrefab(f.directory.path().u8string(), "assets/not-prefab.txt", f.scripts));
        write(fs::u8path(f.path), "[broken");
        CHECK(!library.loadPrefab(f.directory.path().u8string(), f.path, f.scripts));
        write(fs::u8path(f.path), saved);
        auto loaded = library.loadPrefab(f.directory.path().u8string(), f.path, f.scripts);
        CHECK(loaded);
        CHECK(target.instantiateTemplate(loaded -> getEntity(), f.scripts));
    }

    void runtime() {
        Fixture f;
        f.save();
        PrefabManager library;
        auto prefab = library.loadPrefab(f.directory.path().u8string(), f.path, f.scripts);
        CHECK(prefab);
        Scene target{f.bus};
        target.markSaved();
        target.onRuntimeStart(f.scripts);
        auto first = target.instantiateTemplate(prefab -> getEntity(), f.scripts);
        auto second = target.instantiateTemplate(prefab -> getEntity(), f.scripts);
        CHECK(first && second);
        CHECK(target.getRootEntities(SceneContext::Editor).empty());
        CHECK(target.getRootEntities(SceneContext::Runtime).size() == 2);
        CHECK(target.getEntity(first.getUUID(), SceneContext::Runtime) == first);
        CHECK(!target.getEntity(first.getUUID(), SceneContext::Editor));
        CHECK(first.getChildren().front().getParent() == first);
        CHECK(!target.hasChanges());
        CHECK(!target.duplicateEmptyEntity(prefab -> getEntity(), f.scripts));
        target.onRuntimeStop(f.scripts);
        CHECK(!first && !second);
        target.onRuntimeStart(f.scripts);
        CHECK(target.getRootEntities(SceneContext::Runtime).empty());
        CHECK(target.instantiateTemplate(prefab -> getEntity(), f.scripts));
        target.onRuntimeStop(f.scripts);
    }

    void package() {
        Fixture f;
        fs::create_directories(f.directory / "assets/images");
        // A real one-pixel uncompressed TGA; image loading is CPU-only.
        std::string tga(18, '\0');
        tga[2] = 2; tga[12] = 1; tga[14] = 1; tga[16] = 24;
        tga.append(3, char(100));
        write(f.directory / "assets/images/enemy.tga", tga);
        Animation clip;
        clip.name = "Idle"; clip.framesPerSecond = 12; clip.isLooped = true;
        clip.texturePath = "assets/images/enemy.tga";
        clip.filePath = (f.directory / "assets/enemies/idle.anim").u8string();
        clip.addFrame({0, 0, 1, 1});
        AnimationParser parser;
        CHECK(parser.saveToFile(clip.filePath, &clip));
        f.root.addComponent<AnimationComponent>();
        ResourceManager::getManager() -> setAnimationPathsToLoad(f.root.getUUID(), {"assets/enemies/idle.anim"});
        f.save();
        const auto destination = f.directory / "package";
        copyProjectAssets(f.directory.path().u8string(), destination.u8string());
        fs::rename(f.directory / "assets", f.directory / "unavailable-original-assets");
        PrefabManager library;
        auto prefab = library.loadPrefab(destination.u8string(), "assets/enemies/enemy.prefab", f.scripts);
        CHECK(prefab);
        const auto animations = ResourceManager::getManager() -> getAnimationsCopy(prefab -> getEntity().getUUID());
        CHECK(animations.size() == 1 && animations.front().name == "Idle");
        CHECK(animations.front().filePath == (destination / "assets/enemies/idle.anim").u8string());
        CHECK(ResourceManager::getManager() -> hasImage(resourceFileKey((destination /
            "assets/images/enemy.tga").u8string())));
        Scene target{f.bus};
        auto instance = target.instantiateTemplate(prefab -> getEntity(), f.scripts);
        CHECK(instance && instance.getChildren().size() == 1);
        const auto copies = ResourceManager::getManager() -> getAnimationsCopy(instance.getUUID());
        CHECK(copies.size() == 1 && copies.front().name == "Idle");
    }
}

int main(int argc, char** argv) {
    logger::Log::Init();
    try {
        CHECK(argc == 2);
        const std::string scenario = argv[1];
        if(scenario == "instances") instances();
        else if(scenario == "invalid") invalid();
        else if(scenario == "runtime") runtime();
        else if(scenario == "package") package();
        else CHECK(false);
        std::cout << scenario << " passed\n";
        return 0;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
