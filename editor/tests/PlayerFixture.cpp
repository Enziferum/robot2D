#include "TestSupport.hpp"
#include <editor/Scene.hpp>
#include <editor/serializers/SceneSerializer.hpp>
#include <editor/AnimationParser.hpp>
#include <editor/ResouceManager.hpp>
#include <robot2D/Util/Logger.hpp>
#include <iostream>

int main(int argc, char** argv) {
    logger::Log::Init();
    try {
        CHECK(argc == 2);
        const auto root = tests::fs::absolute(tests::fs::u8path(argv[1]));
        tests::fs::create_directories(root / "assets/scenes");
        tests::fs::create_directories(root / "assets/fonts");
        tests::fs::create_directories(root / "assets/scripts/bin");
        tests::fs::create_directories(root / "assets/clips");
        tests::fs::copy_file(tests::fs::path(RB2D_TEST_RESOURCE_DIR) / "fonts/notosans-regular.ttf",
            root / "assets/fonts/hud.ttf", tests::fs::copy_options::overwrite_existing);
        tests::fs::copy_file(tests::fs::path(RB2D_TEST_PLAYER_SCRIPT), root / "assets/scripts/bin/Arena.dll",
            tests::fs::copy_options::overwrite_existing);
        const auto monoConfig = root / "runtime/mono/etc/mono/config";
        auto config = tests::read(monoConfig);
        const auto closingTag = config.rfind("</configuration>");
        CHECK(closingTag != std::string::npos);
        config.insert(closingTag,
            R"(
<dllmap dll="Robot2DPlayerNativeProbe" target="msvcrt.dll" os="windows" />
<dllmap dll="Robot2DPlayerNativeProbe" target="libc.so.6" os="!windows" />
)");
        tests::write(monoConfig, config);
        std::string image(18, '\0'); image[2] = 2; image[12] = 4; image[14] = 2; image[16] = 32; image[17] = 0x28;
        for(int i = 0; i < 8; ++i) image.append("\xff\xff\xff\xff", 4);
        tests::write(root / "assets/sheet.tga", image);
        robot2D::MessageBus bus; auto scene = std::make_shared<editor::Scene>(bus);
        scene -> setAssociatedProjectPath(root.u8string());
        auto camera = scene -> createEntity("Camera");
        auto& view = camera.addComponent<editor::CameraComponent>(); view.isPrimary = true;
        view.position = {0, 0}; view.size = {320, 180};
        camera.addComponent<editor::DrawableComponent>().setColor(robot2D::Color::Transparent);
        const auto box = [&](const std::string& name, robot2D::vec2f pos, robot2D::vec2f size, bool dynamic) {
            auto entity = scene -> createEntity(name);
            entity.getComponent<editor::TransformComponent>().setPosition(pos);
            entity.getComponent<editor::TransformComponent>().setSize(size);
            auto& body = entity.addComponent<editor::Physics2DComponent>();
            body.type =
                dynamic ? editor::Physics2DComponent::BodyType::Dynamic : editor::Physics2DComponent::BodyType::Static;
            body.gravityScale = 0.f; entity.addComponent<editor::Collider2DComponent>();
            entity.addComponent<editor::DrawableComponent>().setColor(robot2D::Color::Blue);
            return entity;
        };
        box("Top", {0, 0}, {320, 8}, false); box("Bottom", {0, 172}, {320, 8}, false);
        box("Left", {0, 0}, {8, 180}, false); box("Right", {312, 0}, {8, 180}, false);
        auto trigger = box("Trigger", {48, 48}, {96, 96}, false);
        trigger.getComponent<editor::Collider2DComponent>().isTrigger = true;
        trigger.getComponent<editor::DrawableComponent>().setColor(robot2D::Color::Transparent);
        auto actor = box("Actor", {64, 64}, {16, 16}, true);
        actor.getComponent<editor::DrawableComponent>().setTexturePath("assets/sheet.tga");
        actor.addComponent<editor::AnimationComponent>(); actor.addComponent<editor::AnimatorComponent>();
        actor.addComponent<editor::ScriptComponent>().name = "PlayerSmoke";
        editor::Animation animation; animation.name = "Idle"; animation.texturePath = "assets/sheet.tga";
        animation.framesPerSecond = 12; animation.isLooped = true;
        animation.addFrame({0, 0, 2, 2}); animation.addFrame({2, 0, 2, 2});
        editor::AnimationParser parser;
        CHECK(parser.saveToFile((root / "assets/clips/idle.anim").u8string(), &animation));
        editor::ResourceManager::getManager() -> setAnimationPathsToLoad(actor.getUUID(), {"assets/clips/idle.anim"});
        auto hud = scene -> createEntity("HUD"); hud.getComponent<editor::TransformComponent>().setPosition({8, 8});
        hud.addComponent<editor::DrawableComponent>().setDepth(10);
        hud.addComponent<editor::TextComponent>().setFontPath("assets/fonts/hud.ttf");
        auto fields = std::make_shared<editor::ScriptInteractor>(editor::EditorInteractor::WeakPtr{}, nullptr);
        auto& field = fields -> getFields(actor.getUUID())["StartingWave"];
        field.setType(editor::FieldType::Int); field.setName("StartingWave"); field.setValue(3);
        editor::SceneSerializer serializer(scene);
        CHECK(serializer.serialize((root / "assets/scenes/main.scene").u8string(), "Arena", fields));
        tests::write(root / "game-launch.yaml",
            "Version: 1\nTitle: Player arena\nStartScene: assets/scenes/main.scene\n"
            "ScriptAssembly: assets/scripts/bin/Arena.dll\nScriptCore: runtime/robot2D_ScriptCore.dll\n"
            "MonoLib: runtime/mono/lib\nMonoEtc: runtime/mono/etc\nWindow: {Width: 640, Height: 360, Fullscreen: false}\n");
        return 0;
    }
    catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
