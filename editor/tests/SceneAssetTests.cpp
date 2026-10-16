#include "TestSupport.hpp"
#include <editor/Scene.hpp>
#include <editor/FileApi.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/serializers/SceneSerializer.hpp>
#include <editor/async/SceneLoadTask.hpp>
#include <robot2D/Util/Logger.hpp>
#include <iostream>

namespace {
    using namespace tests;

    YAML::Node document() {
        return YAML::Load(
            R"(SceneVersion: 1
Scene: Compatibility
EntityCount: 1
RootEntities: [1]
Entities:
  - Entity: 1
    TagComponent: {Tag: Wall}
    TransformComponent: {Position: [0, 0], Size: [20, 20], Rotation: 0, Origin: [0, 0]}
    SpriteComponent: {Color: [255, 255, 255, 255], TexturePath: "assets\\textures\\Wall.tga\x00", zDepth: 1}
    TextComponent: {Text: Label, FontPath: "assets\\fonts\\missing.ttf\x00"}
    AnimationComponent: {Animations: ["assets\\animations\\missing.anim\x00"]}
)");
    }

    void compatibility() {
        Directory fixture;
        fs::create_directories(fixture / "assets/textures");
        std::string image(18, '\0');
        image[2] = 2; image[12] = 1; image[14] = 1; image[16] = 24;
        image.append(3, char(100));
        write(fixture / "assets/textures/Wall.tga", image);
        robot2D::MessageBus bus;
        auto scene = std::make_shared<editor::Scene>(bus);
        scene -> setAssociatedProjectPath(fixture.path().u8string());
        editor::SceneSerializer reader(scene);
        CHECK(reader.deserializeDocument(document(), {}));
        auto entity = scene -> getEntity(editor::UUID{1});
        CHECK(entity.getComponent<editor::DrawableComponent>().getTexturePath() == "assets/textures/Wall.tga");
        CHECK(entity.getComponent<editor::TextComponent>().getFontPath() == "assets/fonts/missing.ttf");
        CHECK(editor::ResourceManager::getManager() -> getAnimationPaths(entity.getUUID()).front() ==
            "assets/animations/missing.anim");
        // Missing resources remain warnings; a trailing legacy terminator must not abort the load.
        editor::loadSceneAssets(scene);
        CHECK(editor::ResourceManager::getManager() -> hasImage(
                editor::resourceFileKey((fixture / "assets/textures/Wall.tga").u8string())));
        const auto saved = fixture / "saved.scene";
        CHECK(reader.serialize(saved.u8string(), "Compatibility", {}));
        CHECK(read(saved).find("\\x00") == std::string::npos);
        auto reopened = std::make_shared<editor::Scene>(bus);
        editor::SceneSerializer second(reopened);
        CHECK(second.deserialize(saved.u8string(), {}));
        CHECK(reopened -> getEntity(editor::UUID{1}).getComponent<editor::DrawableComponent>().getTexturePath() ==
            "assets/textures/Wall.tga");
    }

    void invalid() {
        robot2D::MessageBus bus;
        for(const auto& field: {"TexturePath", "FontPath", "Animations"}) {
            auto data = document();
            auto entity = data["Entities"][0];
            const std::string path = std::string("assets/bad") + char(0) + "suffix.png";
            if(std::string(field) == "TexturePath") entity["SpriteComponent"][field] = path;
            else if(std::string(field) == "FontPath") entity["TextComponent"][field] = path;
            else entity["AnimationComponent"][field][0] = path;
            auto scene = std::make_shared<editor::Scene>(bus);
            editor::SceneSerializer serializer(scene);
            CHECK(!serializer.deserializeDocument(data, {}));
            CHECK(serializer.getErrorMessage().find("Embedded NUL") != std::string::npos);
        }
    }


}

int main(int argc, char** argv) {
    logger::Log::Init();
    try {
        CHECK(argc == 2);
        const std::string scenario = argv[1];
        if(scenario == "compatibility") compatibility();
        else if(scenario == "invalid") invalid();
        else CHECK(false);
        return 0;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
