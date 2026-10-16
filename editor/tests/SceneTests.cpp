#define GLFW_INCLUDE_NONE
#include "TestSupport.hpp"
#include <GLFW/glfw3.h>
#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Util/Logger.hpp>
#include <editor/Scene.hpp>
#include <editor/SceneManager.hpp>
#include <editor/Editor.hpp>
#include <editor/EditorLogic.hpp>
#include <editor/serializers/SceneSerializer.hpp>
#include <editor/serializers/YamlFile.hpp>
#include <editor/serializers/PrefabSerializer.hpp>
#include <editor/FileApi.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/LocalResourceManager.hpp>
#include <editor/TextSystem.hpp>
#include <editor/async/AnimationTextureSliceTask.hpp>
#include <array>
#include <iostream>
#include <limits>

namespace {
    using namespace tests;

    struct GraphicsUnavailable : std::runtime_error { using std::runtime_error::runtime_error; };

    class GraphicsContext {
    public:
        GraphicsContext() {
            if(!glfwInit()) throw GraphicsUnavailable("GLFW initialization unavailable");
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            m_window = glfwCreateWindow(32, 32, "robot2D regression tests", nullptr, nullptr);
            if(!m_window) {
                glfwTerminate();
                throw GraphicsUnavailable("OpenGL 4.5 context unavailable");
            }
            glfwMakeContextCurrent(m_window);
            if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
                glfwDestroyWindow(m_window);
                glfwTerminate();
                throw GraphicsUnavailable("OpenGL loader unavailable");
            }
        }
        ~GraphicsContext() {
            editor::LocalResourceManager::getManager() -> releaseGraphicsResources();
            glfwDestroyWindow(m_window); glfwTerminate();
        }
    private:
        GLFWwindow* m_window { nullptr };
    };

    class ScriptStub : public editor::IScriptInteractorFrom {
    public:
        bool hasEntityClass(const std::string&) const override {
            return false;
        }
        void setScriptClass(std::string, editor::UUID) override {
        }
        bool hasFields(editor::UUID) const override {
            return !fields.empty();
        }
        FieldMap& getFields(editor::UUID) override {
            return fields;
        }
        std::vector<std::string> getClassesNames() const override {
            return {};
        }
        void callScriptMethod(editor::UUID, const std::string&) override {
        }
        void setEditorCamera(editor::EditorCamera::Ptr) override {
        }
        editor::ScriptingEngineService* getScriptingEngine() const override {
            return nullptr;
        }
        FieldMap fields;
    };

    struct QueueStop {
        ~QueueStop() {
            editor::TaskQueue::GetQueue() -> stop();
        }
    };

    std::vector<std::uint64_t> ids(const std::list<editor::SceneEntity>& entities) {
        std::vector<std::uint64_t> result;
        for(auto entity: entities) result.push_back(entity.getUUID());
        return result;
    }

    void assetRoundtrip() {
        Directory fixture;
        robot2D::MessageBus bus;
        auto source = std::make_shared<editor::Scene>(bus);
        source -> setAssociatedProjectPath(fixture.path().u8string());
        auto entity = source -> createEntity("Portable resources");
        entity.addComponent<editor::TextComponent>().setFontPath("assets/fonts/missing.ttf");
        entity.addComponent<editor::DrawableComponent>().setTexturePath(
            (fixture / "assets/textures/missing.png").u8string());
        auto scripts = std::make_shared<ScriptStub>();
        editor::SceneSerializer serializer(source);
        const auto path = fixture / "portable.scene";
        CHECK(serializer.serialize(path.u8string(), "Portable", scripts));
        const auto saved = read(path);
        CHECK(saved.find(fixture.path().u8string()) == std::string::npos);
        CHECK(saved.find("assets/fonts/missing.ttf") != std::string::npos);
        CHECK(saved.find("assets/textures/missing.png") != std::string::npos);
        auto restored = std::make_shared<editor::Scene>(bus);
        restored -> setAssociatedProjectPath((fixture / "moved project").u8string());
        editor::SceneSerializer reader(restored);
        CHECK(reader.deserialize(path.u8string(), scripts));
        const auto loaded = restored -> getEntity(entity.getUUID());
        CHECK(loaded.getComponent<editor::TextComponent>().getFontPath() == "assets/fonts/missing.ttf");
        CHECK(loaded.getComponent<editor::DrawableComponent>().getTexturePath() == "assets/textures/missing.png");
        entity.getComponent<editor::TextComponent>().setFontPath("../external.ttf");
        CHECK(!serializer.serialize(path.u8string(), "Portable", scripts));
        CHECK(read(path) == saved && !serializer.getErrorMessage().empty());
    }

    void runtimeCameraOrder() {
        class Target final : public robot2D::RenderTarget {
        public:
            Target() : RenderTarget({32, 32}, robot2D::WindowContext::RenderApi::OpenGL4_5,
                    robot2D::WindowContext::RenderDimensionType::TwoD) {
            }
            ~Target() override = default;
        } target;
        robot2D::MessageBus bus;
        editor::Scene scene(bus);
        auto sprite = scene.createEntity("Drawn before camera in z order");
        sprite.getComponent<editor::TransformComponent>().setPosition({32, 0});
        sprite.getComponent<editor::TransformComponent>().setSize({8, 8});
        sprite.addComponent<editor::DrawableComponent>().setColor(robot2D::Color::Red);
        auto camera = scene.createEntity("Camera after sprite");
        auto& cameraData = camera.addComponent<editor::CameraComponent>();
        cameraData.isPrimary = true;
        cameraData.position = {16, 0};
        cameraData.size = {64, 64};
        auto& cameraDraw = camera.addComponent<editor::DrawableComponent>();
        cameraDraw.setDepth(10);
        cameraDraw.setReorderZBuffer(true);
        cameraDraw.setColor(robot2D::Color(0, 0, 0, 0));
        scene.update(0);
        scene.setRuntimeCamera(true);
        scene.setRuntimeWindowSize({32, 32});
        target.beforeRender();
        target.clear(robot2D::Color::Black);
        target.draw(scene);
        target.afterRender();
        std::array<unsigned char, 4> pixel {};
        glReadPixels(9, 23, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
        CHECK(pixel[0] > 250 && pixel[1] < 3 && pixel[2] < 3);
        CHECK(glGetError() == GL_NO_ERROR);
    }

    void editorGuiScene() {
        Directory fixture;
        fs::create_directories(fixture / "res");
        for(const auto* directory : {"textures", "shaders", "icons", "fonts"})
            fs::copy(fs::path(RB2D_TEST_RESOURCE_DIR) / directory, fixture.path() / "res" / directory,
                fs::copy_options::recursive);
        struct WorkingDirectory {
            fs::path previous = fs::current_path();
            ~WorkingDirectory() {
                fs::current_path(previous);
            }
        } workingDirectory;
        fs::current_path(fixture.path());
        auto* previousContext = glfwGetCurrentContext();
        robot2D::WindowContext options(false, false, robot2D::WindowContext::RenderApi::OpenGL4_5);
        auto window = std::make_unique<robot2D::RenderWindow>(robot2D::vec2u{1280, 720}, "Editor integration test",
                options);
        struct WindowCleanup {
            std::unique_ptr<robot2D::RenderWindow>& window;
            GLFWwindow* previous;
            ~WindowCleanup() {
                auto* native = static_cast<GLFWwindow*>(window -> getRaw());
                window.reset();
                glfwDestroyWindow(native);
                glfwMakeContextCurrent(previous);
            }
        } windowCleanup{window, previousContext};
        robot2D::Gui gui;
        robot2D::GuiConfiguration configuration;
        configuration.window = window.get();
        configuration.allowDefaultFallback = true;
        CHECK(gui.initialize(configuration));
        robot2D::GuiContextScope scope(gui.context());
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        robot2D::MessageBus bus;
        editor::MessageDispatcher dispatcher;
        editor::Editor view(bus, gui);
        editor::EditorPresenter presenter(&view);
        editor::EditorRouter router(&view);
        editor::EditorLogic logic(bus, dispatcher, presenter, router);
        auto scripts = std::make_shared<ScriptStub>();
        logic.setup(scripts);
        QueueStop stop;
        view.setup(window.get(), &logic);
        auto scene = std::make_shared<editor::Scene>(bus);
        auto camera = scene -> createEntity("Separate game camera");
        auto& cameraData = camera.addComponent<editor::CameraComponent>();
        cameraData.isPrimary = true;
        cameraData.position = {608, 517};
        cameraData.size = {894, 489};
        camera.addComponent<editor::DrawableComponent>().setColor(robot2D::Color(255, 255, 255, 0));
        auto sprite = scene -> createEntity("Visible quad");
        sprite.getComponent<editor::TransformComponent>().setPosition({100, 100});
        sprite.getComponent<editor::TransformComponent>().setSize({100, 100});
        sprite.addComponent<editor::DrawableComponent>().setColor(robot2D::Color::Red);
        scene -> update(0);
        fs::create_directories(fixture / "assets/scenes");
        editor::SceneSerializer serializer(scene);
        CHECK(serializer.serialize((fixture / "assets/scenes/main.scene").u8string(), "Test", scripts));
        auto project = std::make_shared<editor::Project>();
        project -> setPath(fixture.path().u8string());
        project -> setStartScene("main.scene");
        logic.loadProject(project);
        CHECK(waitFor([&]() {
            editor::TaskQueue::GetQueue() -> process();
            return presenter.getState() != editor::EditorState::Load;
        }));
        CHECK(presenter.getState() == editor::EditorState::Edit);
        for(int frame = 0; frame < 5; ++frame) {
            view.update(1.f / 60);
            CHECK(gui.beginFrame(1.f / 60));
            window -> clear();
            view.render();
            CHECK(gui.endFrame());
            window -> afterRender();
            CHECK(gui.render());
            const auto size = window -> getFramebufferSize();
            std::vector<unsigned char> pixels(size.x * size.y * 4);
            glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            std::size_t red = 0;
            for(std::size_t i = 0; i < pixels.size(); i += 4)
                if(pixels[i] > 250 && pixels[i + 1] < 3 && pixels[i + 2] < 3) ++red;
            // Docking assigns panel sizes over the first frames; subsequent frames must contain the scene.
            if(frame >= 3) CHECK(red >= 1000);
            CHECK(glGetError() == GL_NO_ERROR);
        }
        logic.destroy();
        gui.shutdown();
    }

    void hudText() {
        class Target final : public robot2D::RenderTarget {
        public:
            Target() : RenderTarget({32, 32}, robot2D::WindowContext::RenderApi::OpenGL4_5,
                    robot2D::WindowContext::RenderDimensionType::TwoD) {
            }
        } target;
        robot2D::Font regular, bold;
        CHECK(regular.loadFromFile((fs::path(RB2D_TEST_RESOURCE_DIR) / "fonts/notosans-regular.ttf").u8string(), 10));
        CHECK(bold.loadFromFile((fs::path(RB2D_TEST_RESOURCE_DIR) / "fonts/notosans-bold.ttf").u8string(), 10));
        robot2D::MessageBus bus; editor::Scene scene(bus);
        auto background = scene.createEntity("Background");
        background.getComponent<editor::TransformComponent>().setPosition({0, 0});
        background.getComponent<editor::TransformComponent>().setSize({32, 32});
        auto& bg = background.addComponent<editor::DrawableComponent>(); bg.setColor(robot2D::Color::Blue);
        bg.setDepth(0);
        auto first = scene.createEntity("Health"); auto second = scene.createEntity("Wave");
        const auto label = [](editor::SceneEntity entity, const robot2D::Font& font, float x, robot2D::Color color) {
            entity.getComponent<editor::TransformComponent>().setPosition({x, 2});
            auto& drawable = entity.addComponent<editor::DrawableComponent>(); drawable.setColor(color);
            drawable.setDepth(1);
            auto& text = entity.addComponent<editor::TextComponent>(); text.setFont(font); text.setText("H");
        };
        label(first, regular, 1, robot2D::Color::Red); label(second, bold, 17, robot2D::Color::Green);
        auto missing = scene.createEntity("No font yet"); missing.addComponent<editor::DrawableComponent>();
        missing.addComponent<editor::TextComponent>().setText("Not loaded");
        const auto render = [&]() {
            // GLFW's framebuffer may be larger than 32 pixels on a HiDPI desktop.
            glViewport(0, 0, 32, 32);
            scene.update(0); target.beforeRender(); target.clear(robot2D::Color::Black); target.draw(scene);
            target.afterRender();
            std::array<unsigned char, 32 * 32 * 4> pixels{};
            glReadPixels(0, 0, 32, 32, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            std::array<int, 2> colors{};
            for(std::size_t i = 0; i < pixels.size(); i += 4) {
                if(pixels[i] > 80 && pixels[i + 1] < 10) ++colors[0];
                if(pixels[i + 1] > 80 && pixels[i] < 10) ++colors[1];
            }
            CHECK(glGetError() == GL_NO_ERROR); return colors;
        };
        auto colors = render();
        CHECK(colors[0] > 3 && colors[1] > 3);
        auto cover = scene.createEntity("Cover health only");
        cover.getComponent<editor::TransformComponent>().setPosition({0, 0});
        cover.getComponent<editor::TransformComponent>().setSize({16, 32});
        auto& overlay = cover.addComponent<editor::DrawableComponent>(); overlay.setColor(robot2D::Color::Blue);
        overlay.setDepth(2); overlay.setReorderZBuffer(true);
        colors = render(); CHECK(colors[0] == 0 && colors[1] > 3);
        second.getComponent<editor::TextComponent>().setText(""); colors = render(); CHECK(colors[1] == 0);
        second.getComponent<editor::TextComponent>().setText(std::string(2001, 'H')); render();
        second.getComponent<editor::TextComponent>().setText(u8"Игрок\nH"); render();
        second.getComponent<editor::TextComponent>().setFont(regular); render();
        CHECK(first.getComponent<editor::TransformComponent>().getPosition().x == 1);
        std::weak_ptr<robot2D::Texture> texture = first.getComponent<editor::TextComponent>().getTextureHandle();
        first.removeComponent<editor::TextComponent>(); render();
        CHECK(!texture.expired());
        CHECK(first.getComponent<editor::DrawableComponent>().getTexture().getSize().x > 0);
        first.getComponent<editor::DrawableComponent>().setTexture(std::shared_ptr<robot2D::Texture>{});
        CHECK(!first.getComponent<editor::DrawableComponent>().hasTexture());
        robot2D::Font empty;
        second.getComponent<editor::TextComponent>().setFont(empty); render();
        CHECK(!second.getComponent<editor::DrawableComponent>().hasTexture());
    }

    void hierarchy() {
        robot2D::MessageBus bus;
        editor::Scene scene(bus);
        auto a = scene.createEntity("Parent");
        auto b = scene.createEntity("Child");
        auto c = scene.createEntity("Grandchild");
        auto d = scene.createEntity("Other root");
        CHECK(scene.reparent(b, a) && scene.reparent(c, b));
        CHECK(b.getParent() == a && c.getParent() == b);
        CHECK(scene.getRootEntities().size() == 2);
        CHECK(!scene.reparent(a, c) && !scene.reparent(a, a));
        CHECK(scene.setBefore(d, a));
        CHECK(ids(scene.getRootEntities()) == std::vector<std::uint64_t>({d.getUUID(), a.getUUID()}));
        CHECK(scene.reparent(c, {}));
        CHECK(!c.getParent() && b.getChildren().empty());
        editor::Scene foreign(bus);
        CHECK(!foreign.reparent(b, a));
        auto duplicate = scene.createEmptyEntity();
        duplicate.addComponent<editor::IDComponent>().ID = a.getUUID();
        CHECK(!scene.attachEntity(duplicate));
        CHECK(scene.getEntity(a.getUUID()) == a);
    }

    void roundtrip() {
        Directory fixture;
        robot2D::MessageBus bus;
        auto source = std::make_shared<editor::Scene>(bus);
        auto a = source -> createEntity("Parent % ###");
        auto b = source -> createEntity("Child");
        auto c = source -> createEntity("Other root");
        CHECK(source -> reparent(b, a));
        CHECK(source -> setBefore(c, a));
        b.getComponent<editor::TransformComponent>().setPosition({17, 31});
        b.getComponent<editor::TransformComponent>().setRotate(37.f);
        auto& drawable = a.addComponent<editor::DrawableComponent>();
        drawable.setTexturePath("assets/textures/example.png");
        drawable.setDepth(7);
        const auto file = fixture / "roundtrip.scene";
        editor::SceneSerializer writer(source);
        CHECK(writer.serialize(file.u8string(), "Named scene", nullptr));
        auto document = YAML::Load(read(file));
        CHECK(document["SceneVersion"].as<int>() == 1 && document["EntityCount"].as<int>() == 3);
        YAML::Node reversed(YAML::NodeType::Sequence);
        for(std::size_t i = document["Entities"].size(); i > 0; --i)
            reversed.push_back(document["Entities"][i - 1]);
        document["Entities"] = reversed;
        write(file, YAML::Dump(document));
        auto loaded = std::make_shared<editor::Scene>(bus);
        editor::SceneSerializer reader(loaded);
        CHECK(reader.deserialize(file.u8string(), nullptr));
        CHECK(loaded -> getName() == "Named scene" && !loaded -> hasChanges());
        CHECK(ids(loaded -> getRootEntities()) == ids(source -> getRootEntities()));
        CHECK(loaded -> getEntity(b.getUUID()).getParent() == loaded -> getEntity(a.getUUID()));
        CHECK(loaded -> getEntity(b.getUUID()).getComponent<editor::TransformComponent>().getPosition()
            == robot2D::vec2f(17, 31));
        CHECK(loaded -> getEntity(a.getUUID()).getComponent<editor::DrawableComponent>().getDepth() == 7);
        CHECK(!reader.deserialize(file.u8string(), nullptr));
        CHECK(reader.getError() == editor::SceneSerializerError::InvalidScene);
    }

    void invalid() {
        Directory fixture;
        robot2D::MessageBus bus;
        auto source = std::make_shared<editor::Scene>(bus);
        auto a = source -> createEntity("Parent");
        auto b = source -> createEntity("Child");
        CHECK(source -> reparent(b, a));
        const auto file = fixture / "scene.scene";
        editor::SceneSerializer writer(source);
        CHECK(writer.serialize(file.u8string(), "Valid", nullptr));
        const auto valid = YAML::Load(read(file));
        auto reject = [&](const YAML::Node& document, editor::SceneSerializerError expected) {
            auto target = std::make_shared<editor::Scene>(bus);
            editor::SceneSerializer reader(target);
            CHECK(!reader.deserializeDocument(document, nullptr));
            CHECK(reader.getError() == expected && !reader.getErrorMessage().empty());
        };
        auto document = YAML::Clone(valid);
        document["SceneVersion"] = 999;
        reject(document, editor::SceneSerializerError::UnsupportedVersion);
        document = YAML::Clone(valid);
        document["EntityCount"] = 999;
        reject(document, editor::SceneSerializerError::CountMismatch);
        document = YAML::Clone(valid);
        document["RootEntities"].push_back(document["RootEntities"][0]);
        reject(document, editor::SceneSerializerError::InvalidHierarchy);
        document = YAML::Clone(valid);
        document["Entities"][0]["Entity"] = document["Entities"][1]["Entity"].as<std::uint64_t>();
        reject(document, editor::SceneSerializerError::InvalidData);
        document = YAML::Clone(valid);
        document["Entities"][0]["HierarchyComponent"]["Children"].push_back(static_cast<std::uint64_t>(a.getUUID()));
        reject(document, editor::SceneSerializerError::InvalidHierarchy);
        for(const auto* text: {"[broken", "[]", "Scene: &self {loop: *self}\nEntities: []\n"}) {
            write(file, text);
            auto target = std::make_shared<editor::Scene>(bus);
            editor::SceneSerializer reader(target);
            CHECK(!reader.deserialize(file.u8string(), nullptr) && !reader.getErrorMessage().empty());
        }
    }

    void legacy() {
        Directory fixture;
        const auto file = fixture / "legacy.scene";
        write(file, "Scene: Legacy\nEntities:\n- Entity: 100\n  Entity: 100\n  TagComponent: {Tag: Parent}\n"
            "  TransformComponent:\n    Position: [0, 0]\n    Size: [1, 1]\n    Rotation: 0\n"
            "    HasChildren: true\n    ChildIDs: [200]\n- Entity: 200\n  TagComponent: {Tag: Child}\n"
            "  TransformComponent:\n    Position: [0, 0]\n    Size: [1, 1]\n    Rotation: 0\n"
            "    isChild: true\n    ParentID: 100\n");
        robot2D::MessageBus bus;
        auto scene = std::make_shared<editor::Scene>(bus);
        editor::SceneSerializer reader(scene);
        CHECK(reader.deserialize(file.u8string(), nullptr));
        CHECK(scene -> getName() == "Legacy" && !scene -> hasChanges());
        CHECK(scene -> getEntity(editor::UUID{200}).getParent() == scene -> getEntity(editor::UUID{100}));
    }

    void saveDirty() {
        Directory fixture;
        robot2D::MessageBus bus;
        auto scripts = std::make_shared<ScriptStub>();
        auto scene = std::make_shared<editor::Scene>(bus);
        auto entity = scene -> createEntity("Object");
        scene -> setPath((fixture / "saved.scene").u8string());
        editor::SceneManager manager(bus);
        CHECK(manager.save(editor::Scene::Ptr(scene), scripts));
        CHECK(!scene -> hasChanges());
        const auto original = read(fixture / "saved.scene");
        const auto originalPath = scene -> getPath();
        entity.markModified();
        CHECK(scene -> hasChanges());
        CHECK(!manager.save(editor::Scene::Ptr(scene), {}));
        CHECK(manager.getError() == editor::SceneManagerError::SceneSerialize);
        CHECK(scene -> hasChanges() && scene -> getPath() == originalPath);
        CHECK(read(fixture / "saved.scene") == original);
        const auto rejectedLog = read(logger::Log::getFilePath());
        CHECK(rejectedLog.find(originalPath) != std::string::npos);
        CHECK(rejectedLog.find("reason=script interactor unavailable") != std::string::npos);
        CHECK(!manager.save(editor::Scene::Ptr(scene), scripts, (fixture / "missing/failed.scene").u8string()));
        CHECK(scene -> hasChanges() && scene -> getPath() == originalPath);
        CHECK(read(fixture / "saved.scene") == original);
        CHECK(manager.save(editor::Scene::Ptr(scene), scripts, (fixture / "save-as.scene").u8string()));
        CHECK(manager.getError() == editor::SceneManagerError::None);
        CHECK(!scene -> hasChanges() && scene -> getPath() != originalPath);
        auto& transform = entity.getComponent<editor::TransformComponent>();
        transform.setRotate(std::numeric_limits<float>::quiet_NaN());
        editor::SceneSerializer writer(scene);
        CHECK(!writer.serialize((fixture / "saved.scene").u8string(), "Invalid", nullptr));
        CHECK(read(fixture / "saved.scene") == original);
        const auto rejectedPath = (fixture / "no-scene.scene").u8string();
        CHECK(!manager.save({}, scripts, rejectedPath));
        CHECK(manager.getError() == editor::SceneManagerError::SceneSerialize);
        CHECK(!fs::exists(fs::u8path(rejectedPath)));
        const auto noSceneLog = read(logger::Log::getFilePath());
        CHECK(noSceneLog.find(rejectedPath) != std::string::npos);
        CHECK(noSceneLog.find("reason=no active scene") != std::string::npos);
    }

    void prefab() {
        Directory fixture;
        robot2D::MessageBus bus;
        auto source = std::make_shared<editor::Scene>(bus);
        auto parent = source -> createEntity("Template parent");
        auto child = source -> createEntity("Template child");
        CHECK(source -> reparent(child, parent));
        auto prefab = std::make_shared<editor::Prefab>((fixture / "tree.prefab").u8string(),
                parent, editor::UUID{9876});
        editor::PrefabSerializer serializer;
        CHECK(serializer.serialize(prefab, nullptr));
        auto loaded = std::make_shared<editor::Prefab>();
        loaded -> setPath(prefab -> getPath());
        CHECK(serializer.deserialize(loaded, nullptr));
        CHECK(loaded -> getEntity().getName() == "Template parent");
        CHECK(loaded -> getEntity().getChildren().size() == 1);
        CHECK(loaded -> getEntity().getChildren().front().getName() == "Template child");
        CHECK(loaded -> getEntity().getUUID() != parent.getUUID());
        CHECK(source -> getRootEntities().size() == 1 && child.getParent() == parent);
        write(fixture / "bad.prefab", "[broken");
        loaded -> setPath((fixture / "bad.prefab").u8string());
        CHECK(!serializer.deserialize(loaded, nullptr));
        CHECK(source -> getEntity(parent.getUUID()) == parent);
    }

    void asyncLoad() {
        Directory fixture;
        robot2D::MessageBus bus;
        auto scripts = std::make_shared<ScriptStub>();
        auto project = std::make_shared<editor::Project>();
        project -> setPath(fixture.path().u8string());
        auto source = std::make_shared<editor::Scene>(bus);
        editor::SceneSerializer writer(source);
        CHECK(writer.serialize((fixture / "first.scene").u8string(), "First", scripts));
        CHECK(writer.serialize((fixture / "second.scene").u8string(), "Second", scripts));
        editor::SceneManager manager(bus);
        QueueStop stop;
        auto* queue = editor::TaskQueue::GetQueue();
        int staleCalls = 0;
        bool loaded = false;
        manager.loadSceneAsync(project, (fixture / "first.scene").u8string(),
            [&](auto) { ++staleCalls; }, scripts);
        manager.loadSceneAsync(project, (fixture / "second.scene").u8string(),
            [&](auto scene) { CHECK(scene && scene -> getName() == "Second"); loaded = true; }, scripts);
        CHECK(waitFor([&]() { queue -> process(); return loaded; }));
        CHECK(staleCalls == 0);
        const auto previous = manager.getActiveScene();
        bool failed = false;
        manager.loadSceneAsync(project, (fixture / "missing.scene").u8string(),
            [&](auto scene) { CHECK(!scene); failed = true; }, scripts);
        CHECK(waitFor([&]() { queue -> process(); return failed; }));
        CHECK(manager.getActiveScene() == previous);
        bool unavailable = false;
        manager.loadSceneAsync(project, (fixture / "first.scene").u8string(),
            [&](auto scene) { CHECK(!scene); unavailable = true; }, {});
        CHECK(waitFor([&]() { queue -> process(); return unavailable; }));
        CHECK(manager.getActiveScene() == previous);
    }

    class View : public editor::IEditor {
    public:
        editor::Scene::Ptr scene;
        editor::PopupConfiguration* popup { nullptr };
        bool assetDirty { false };
        bool assetSaveOK { true };
        int discarded { 0 };
        void openScene(editor::Scene::Ptr value, std::string) override {
            scene = std::move(value);
        }
        void prepareView() override {
        }
        void clearSelectionOnUI() override {
        }
        void showPopup(editor::PopupConfiguration* value) override {
            popup = value;
        }
        void setMainCameraEntity(editor::SceneEntity) override {
        }
        bool hasAssetChanges() const override {
            return assetDirty;
        }
        bool saveAssetChanges() override {
            if(!assetSaveOK) return false; assetDirty = false; return true;
        }
        void discardAssetChanges() override {
            assetDirty = false; ++discarded;
        }
    };

    void closeProject() {
        Directory fixture;
        fs::create_directories(fixture / "assets/scenes");
        robot2D::MessageBus bus;
        editor::MessageDispatcher dispatcher;
        auto scripts = std::make_shared<ScriptStub>();
        View view;
        editor::EditorPresenter presenter(&view);
        editor::EditorRouter router(&view);
        editor::EditorLogic logic(bus, dispatcher, presenter, router);
        logic.setup(scripts);
        QueueStop stop;
        auto project = std::make_shared<editor::Project>();
        project -> setPath(fixture.path().u8string());
        project -> setStartScene("start.scene");
        auto source = std::make_shared<editor::Scene>(bus);
        editor::SceneSerializer writer(source);
        CHECK(writer.serialize((fixture / "assets/scenes/start.scene").u8string(), "Start", scripts));
        logic.loadProject(project);
        CHECK(waitFor([&]() {
            editor::TaskQueue::GetQueue() -> process();
            return presenter.getState() == editor::EditorState::Edit;
        }));
        CHECK(view.scene);
        int closed = 0;
        view.scene -> markModified();
        logic.closeCurrentProject([&]() { ++closed; });
        CHECK(view.popup && closed == 0);
        // Cancel is represented by dismissing the prompt without choosing Yes/No.
        CHECK(view.scene -> hasChanges() && closed == 0);
        view.scene -> setPath((fixture / "missing/fail.scene").u8string());
        CHECK(!view.popup -> onYes());
        CHECK(closed == 0 && view.scene -> hasChanges() && !view.popup -> error.empty());
        view.scene -> setPath((fixture / "saved.scene").u8string());
        CHECK(view.popup -> onYes());
        CHECK(closed == 1 && !view.scene -> hasChanges());
        presenter.switchState(editor::EditorState::Edit);
        view.assetDirty = true;
        view.assetSaveOK = false;
        logic.closeCurrentProject([&]() { ++closed; });
        CHECK(!view.popup -> onYes() && closed == 1 && view.assetDirty);
        view.popup -> onNo();
        CHECK(closed == 2 && view.discarded == 1 && !view.assetDirty);
    }

    void startupFailure() {
        Directory fixture;
        robot2D::MessageBus bus;
        editor::MessageDispatcher dispatcher;
        auto scripts = std::make_shared<ScriptStub>();
        View view;
        editor::EditorPresenter presenter(&view);
        editor::EditorRouter router(&view);
        editor::EditorLogic logic(bus, dispatcher, presenter, router);
        logic.setup(scripts);
        QueueStop stop;
        auto project = std::make_shared<editor::Project>();
        project -> setPath(fixture.path().u8string());
        project -> setStartScene("absent.scene");
        logic.loadProject(project);
        CHECK(waitFor([&]() {
            editor::TaskQueue::GetQueue() -> process();
            return presenter.getState() != editor::EditorState::Load;
        }));
        CHECK(presenter.getState() == editor::EditorState::LoadFailed && !view.scene);
    }

    void missingAssets() {
        Directory fixture;
        robot2D::MessageBus bus;
        editor::MessageDispatcher dispatcher;
        auto scripts = std::make_shared<ScriptStub>();
        View view;
        editor::EditorPresenter presenter(&view);
        editor::EditorRouter router(&view);
        editor::EditorLogic logic(bus, dispatcher, presenter, router);
        logic.setup(scripts);
        QueueStop stop;
        auto project = std::make_shared<editor::Project>();
        project -> setPath(fixture.path().u8string());
        project -> setStartScene("missing-assets.scene");
        fs::create_directories(fixture / "assets/scenes");
        auto source = std::make_shared<editor::Scene>(bus);
        source -> setAssociatedProjectPath(fixture.path().u8string());
        auto entity = source -> createEntity("Missing resources");
        entity.addComponent<editor::TextComponent>().setFontPath("assets/missing-font.ttf");
        entity.addComponent<editor::DrawableComponent>().setTexturePath("assets/missing-image.png");
        editor::SceneSerializer writer(source);
        CHECK(writer.serialize((fixture / "assets/scenes/missing-assets.scene").u8string(), "Missing", scripts));
        logic.loadProject(project);
        CHECK(waitFor([&]() {
            editor::TaskQueue::GetQueue() -> process();
            return presenter.getState() != editor::EditorState::Load;
        }));
        CHECK(presenter.getState() == editor::EditorState::Edit && view.scene);
        auto loaded = view.scene -> getEntity(entity.getUUID());
        CHECK(!loaded.getComponent<editor::TextComponent>().getFont());
        CHECK(!loaded.getComponent<editor::DrawableComponent>().hasTexture());
        CHECK(loaded.getComponent<editor::TextComponent>().getFontPath() == "assets/missing-font.ttf");
        CHECK(glGetError() == GL_NO_ERROR);
    }

    void writeImage(const fs::path& path, unsigned width, std::uint8_t red) {
        fs::create_directories(path.parent_path());
        // Uncompressed 32-bit TGA, kept local to the fixture; Image::save is a stub.
        std::string bytes(18, '\0');
        bytes[2] = 2;
        bytes[12] = static_cast<char>(width);
        bytes[14] = 2;
        bytes[16] = 32;
        bytes[17] = 0x28;
        for(unsigned pixel = 0; pixel < width * 2; ++pixel) {
            bytes += static_cast<char>(255);
            bytes += static_cast<char>(255);
            bytes += static_cast<char>(red);
            bytes += static_cast<char>(255);
        }
        write(path, bytes);
    }

    void assetIdentity() {
        Directory fixture;
        robot2D::MessageBus bus;
        editor::MessageDispatcher dispatcher;
        auto scripts = std::make_shared<ScriptStub>();
        View view;
        editor::EditorPresenter presenter(&view);
        editor::EditorRouter router(&view);
        editor::EditorLogic logic(bus, dispatcher, presenter, router);
        logic.setup(scripts);
        QueueStop stop;
        auto source = std::make_shared<editor::Scene>(bus);
        auto first = source -> createEntity("First");
        first.addComponent<editor::DrawableComponent>().setTexturePath("assets/a/player.tga");
        first.addComponent<editor::TextComponent>().setFontPath("assets/a/hud.ttf");
        auto second = source -> createEntity("Second");
        second.addComponent<editor::DrawableComponent>().setTexturePath("assets/b/player.tga");
        second.addComponent<editor::TextComponent>().setFontPath("assets/b/hud.ttf");
        auto shared = source -> createEntity("Shared");
        shared.addComponent<editor::DrawableComponent>().setTexturePath("assets/a/../a/player.tga");
        for(unsigned projectIndex = 0; projectIndex < 2; ++projectIndex) {
            const auto root = fixture.path() / ("project-" + std::to_string(projectIndex));
            writeImage(root / "assets/a/player.tga", 2 + projectIndex * 2, 50);
            writeImage(root / "assets/b/player.tga", 3 + projectIndex * 2, 200);
            fs::copy_file(fs::path(RB2D_TEST_RESOURCE_DIR) / "fonts/notosans-regular.ttf", root / "assets/a/hud.ttf");
            fs::copy_file(fs::path(RB2D_TEST_RESOURCE_DIR) / "fonts/notosans-bold.ttf", root / "assets/b/hud.ttf");
            fs::create_directories(root / "assets/scenes");
            source -> setAssociatedProjectPath(root.u8string());
            editor::SceneSerializer writer(source);
            CHECK(writer.serialize((root / "assets/scenes/main.scene").u8string(), "Resources", scripts));
            auto project = std::make_shared<editor::Project>();
            project -> setPath(root.u8string());
            project -> setStartScene("main.scene");
            // Reopen twice in the same process, then load another project with the same UUIDs/filenames.
            for(int reopen = 0; reopen < 2; ++reopen) {
                logic.loadProject(project);
                CHECK(waitFor([&]() {
                    editor::TaskQueue::GetQueue() -> process();
                    return presenter.getState() != editor::EditorState::Load;
                }));
                CHECK(presenter.getState() == editor::EditorState::Edit && view.scene);
                const auto a = view.scene -> getEntity(first.getUUID());
                const auto b = view.scene -> getEntity(second.getUUID());
                const auto alias = view.scene -> getEntity(shared.getUUID());
                const auto& textureA = a.getComponent<editor::DrawableComponent>().getTexture();
                const auto& textureB = b.getComponent<editor::DrawableComponent>().getTexture();
                CHECK(textureA.getSize().x == 2 + projectIndex * 2);
                CHECK(textureB.getSize().x == 3 + projectIndex * 2);
                CHECK(&textureA == alias.getComponent<editor::DrawableComponent>().getTexturePointer());
                const auto* fontA = a.getComponent<editor::TextComponent>().getFont();
                const auto* fontB = b.getComponent<editor::TextComponent>().getFont();
                CHECK(fontA && fontB && fontA != fontB);
                CHECK(fontA -> getPath() == editor::resourceFileKey((root / "assets/a/hud.ttf").u8string()));
                CHECK(fontB -> getPath() == editor::resourceFileKey((root / "assets/b/hud.ttf").u8string()));
                CHECK(glGetError() == GL_NO_ERROR);
            }
        }
        // Cache objects are static; release GL ownership before destroying the context.
        view.scene.reset();
        logic.destroy();
        editor::LocalResourceManager::getManager() -> releaseGraphicsResources();
        CHECK(glGetError() == GL_NO_ERROR);
    }

    void prefabViewport() {
        Directory fixture;
        fs::create_directories(fixture / "assets/scenes");
        fs::create_directories(fixture / "assets/prefabs");
        robot2D::MessageBus bus;
        editor::MessageDispatcher dispatcher;
        auto scripts = std::make_shared<ScriptStub>();
        View view;
        editor::EditorPresenter presenter(&view);
        editor::EditorRouter router(&view);
        editor::EditorLogic logic(bus, dispatcher, presenter, router);
        logic.setup(scripts);
        QueueStop stop;
        auto source = std::make_shared<editor::Scene>(bus);
        auto root = source -> createEntity("Enemy");
        auto child = source -> createEntity("Weapon");
        root.addComponent<editor::DrawableComponent>();
        CHECK(source -> reparent(child, root));
        auto asset = std::make_shared<editor::Prefab>((fixture / "assets/prefabs/enemy.prefab").u8string(),
                root, editor::UUID{9800});
        editor::PrefabSerializer serializer;
        CHECK(serializer.serialize(asset, scripts));
        auto empty = std::make_shared<editor::Scene>(bus);
        editor::SceneSerializer writer(empty);
        CHECK(writer.serialize((fixture / "assets/scenes/main.scene").u8string(), "Empty", scripts));
        auto project = std::make_shared<editor::Project>();
        project -> setPath(fixture.path().u8string());
        project -> setStartScene("main.scene");
        logic.loadProject(project);
        CHECK(waitFor([&]() {
            editor::TaskQueue::GetQueue() -> process();
            return presenter.getState() != editor::EditorState::Load;
        }));
        CHECK(view.scene && view.scene -> getRootEntities().empty());
        const auto send = [&](const std::string& path) {
            editor::Buffer buffer{bus.postMessage(editor::MessageID::PrefabLoad,
                                      editor::StringBuffer::calcAllocSize(path))};
            editor::pack_message_string(path, buffer);
            robot2D::Message message;
            while(bus.pollMessages(message))
                dispatcher.process(message);
        };
        send("assets/prefabs/enemy.prefab");
        CHECK(view.scene -> getRootEntities().size() == 1);
        const auto first = logic.getSelectedEntities().front();
        CHECK(first.getChildren().size() == 1);
        CHECK(first.hasComponent<editor::QuadTreeComponent>());
        CHECK(first.getChildren().front().hasComponent<editor::QuadTreeComponent>());
        send("assets/prefabs/enemy.prefab");
        CHECK(view.scene -> getRootEntities().size() == 2);
        const auto second = logic.getSelectedEntities().front();
        CHECK(first != second && first.getChildren().front() != second.getChildren().front());
        send("assets/prefabs/missing.prefab");
        CHECK(view.scene -> getRootEntities().size() == 2);
        CHECK(logic.getSelectedEntities().front() == second);
        CHECK(view.scene -> hasChanges());
        logic.destroy();
    }

    void animationResources() {
        Directory fixture;
        const auto first = fixture / "assets/a/sheet.tga";
        const auto second = fixture / "assets/b/sheet.tga";
        writeImage(first, 2, 50);
        writeImage(second, 4, 200);
        // A second slice of the same sheet used to dereference a null addImage result.
        for(const auto& path: {first, first, second}) {
            editor::AnimationTextureSliceTask task(nullptr, "sheet.tga", path.u8string());
            task.execute();
            CHECK(!task.getRects().empty());
        }
        auto* manager = editor::ResourceManager::getManager();
        CHECK(manager -> getImage(editor::resourceFileKey(first.u8string())).getSize().x == 2);
        CHECK(manager -> getImage(editor::resourceFileKey(second.u8string())).getSize().x == 4);
        const auto recovered = fixture / "assets/recovered.tga";
        const auto key = editor::resourceFileKey(recovered.u8string());
        CHECK(!manager -> loadImage(key) && !manager -> hasImage(key));
        writeImage(recovered, 3, 100);
        CHECK(manager -> loadImage(key));
        CHECK(manager -> getImage(key).getSize().x == 3);
        CHECK(glGetError() == GL_NO_ERROR);
    }
}

int main(int argc, char** argv) {
    logger::Log::Init();
    try {
        CHECK(argc == 2);
        GraphicsContext graphics;
        const std::string name = argv[1];
        if(name == "hierarchy") hierarchy();
        else if(name == "runtime-camera-order") runtimeCameraOrder();
        else if(name == "editor-gui-scene") editorGuiScene();
        else if(name == "hud-text") hudText();
        else if(name == "asset-roundtrip") assetRoundtrip();
        else if(name == "missing-assets") missingAssets();
        else if(name == "asset-identity") assetIdentity();
        else if(name == "animation-resources") animationResources();
        else if(name == "prefab-viewport") prefabViewport();
        else if(name == "roundtrip") roundtrip();
        else if(name == "invalid") invalid();
        else if(name == "legacy") legacy();
        else if(name == "prefab") prefab();
        else if(name == "save-dirty") saveDirty();
        else if(name == "async-load") asyncLoad();
        else if(name == "close-project") closeProject();
        else if(name == "startup-failure") startupFailure();
        else CHECK(false && "Unknown test case");
        std::cout << name << " passed\n";
        return 0;
    }
    catch(const GraphicsUnavailable& error) {
        std::cerr << "SKIP: " << error.what() << '\n';
        return 77;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
