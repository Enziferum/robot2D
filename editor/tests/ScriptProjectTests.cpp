#include "TestSupport.hpp"
#include <editor/scripting/ScriptProject.hpp>
#include <editor/scripting/ScriptingEngine.hpp>
#include <editor/Scene.hpp>
#include <robot2D/Util/Logger.hpp>
#include <fstream>
#include <iostream>

namespace {
    using namespace editor;
    using namespace tests;
    ScriptProjectTools tools() {
        return {RB2D_TEST_SCRIPT_PYTHON, RB2D_TEST_SCRIPT_GENERATOR, RB2D_TEST_SCRIPTCORE,
                RB2D_TEST_SCRIPT_MCS, RB2D_TEST_SCRIPT_MONO};
    }
    void source(const fs::path& file, const std::string& text) {
        std::ofstream stream(file); stream << text; CHECK(stream.good());
    }
    void project() {
        Directory directory;
        const auto root = directory / "project & spaces";
        const auto sources = root / "author sources";
        fs::create_directories(sources);
        source(sources / "Player.cs", "public sealed class Player : robot2D.Entity {} ");
        auto result = generateProjectScripts(root, "Game & example", sources, tools());
        CHECK(result.success && fs::is_regular_file(result.assembly));
        CHECK(fs::is_regular_file(sources / "Game & example.csproj"));
        source(sources / "Player.cs", "invalid C# source");
        result = buildProjectScripts(root, "Game & example", tools());
        CHECK(!result.success && result.assembly.empty());
        CHECK(result.diagnostic.find("Player.cs") != std::string::npos);
        CHECK(!fs::exists(root / "assets/scripts/bin/Game & example.dll"));
        source(sources / "Player.cs", "public sealed class Player : robot2D.Entity {} ");
        CHECK(buildProjectScripts(root, "Game & example", tools()).success);
    }
    void errors() {
        Directory directory;
        auto configured = tools(); configured.python = directory / "no-python";
        auto result = buildProjectScripts(directory.path(), "Game", configured);
        CHECK(!result.success && !result.diagnostic.empty() && result.assembly.empty());
        configured = tools(); configured.compiler = directory / "no-mcs";
        result = buildProjectScripts(directory.path(), "Game", configured);
        CHECK(!result.success && !result.diagnostic.empty());
    }
    void reload() {
        Directory directory;
        const auto root = directory / "game";
        const auto sources = root / "assets/scripts"; fs::create_directories(sources);
        auto code = sources / "Probe.cs";
        source(code, "public sealed class Probe : robot2D.Entity { public int Value = 1; }");
        auto built = generateProjectScripts(root, "ReloadGame", sources, tools()); CHECK(built.success);
        ScriptEngine engine;
        auto coreFile = directory / "robot2D_ScriptCore.dll";
        fs::copy_file(fs::path(RB2D_TEST_SCRIPTCORE), coreFile);
        auto core = coreFile; core.replace_extension(); engine.Init(core.u8string());
        CHECK(engine.runtimeInit(built.assembly.u8string()));
        robot2D::MessageBus bus; Scene scene(bus); auto entity = scene.createEntity("Probe");
        entity.addComponent<ScriptComponent>().name = "Probe";
        auto scripts = std::make_shared<ScriptInteractor>(EditorInteractor::WeakPtr{}, &engine);
        engine.onRuntimeStart(scripts); engine.onCreateEntity(entity);
        auto instance = engine.getEntityScriptInstance(entity.getUUID()); CHECK(instance);
        int value = 0; CHECK(instance -> getFieldValueInternal("Value", &value) && value == 1);
        CHECK(!engine.runtimeInit(built.assembly.u8string())); // Never unload an active runtime.
        engine.onRuntimeStop(); instance.reset();
        source(code, "public sealed class Probe : robot2D.Entity { public int Value = 2; }");
        built = buildProjectScripts(root, "ReloadGame", tools()); CHECK(built.success);
        CHECK(engine.runtimeInit(built.assembly.u8string()));
        engine.onRuntimeStart(scripts); engine.onCreateEntity(entity);
        instance = engine.getEntityScriptInstance(entity.getUUID()); CHECK(instance);
        CHECK(instance -> getFieldValueInternal("Value", &value) && value == 2);
        engine.onRuntimeStop(); instance.reset();
        source(code, "public class Renamed : robot2D.Entity {} ");
        built = buildProjectScripts(root, "ReloadGame", tools()); CHECK(built.success);
        CHECK(engine.runtimeInit(built.assembly.u8string()));
        CHECK(!engine.hasEntityClass("Probe") && engine.hasEntityClass("Renamed"));
        CHECK(!engine.runtimeInit((root / "missing.dll").u8string()));
        CHECK(!engine.hasEntityClass("Renamed"));
        fs::remove(coreFile);
        CHECK(!engine.runtimeInit(built.assembly.u8string()));
        CHECK(engine.GetCoreAssemblyImage() == nullptr && engine.getClasses().empty());
        scripts.reset(); ScriptGlue::setService(nullptr); engine.Shutdown();
    }
}
int main(int argc, char** argv) {
    logger::Log::Init();
    try {
        CHECK(argc == 2); const std::string mode = argv[1];
        if(mode == "project") project(); else if(mode == "errors") errors(); else if(mode == "reload") reload();
        else CHECK(false);
        return 0;
    }
    catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
