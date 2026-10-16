#include "TestSupport.hpp"
#include <editor/Scene.hpp>
#include <editor/scripting/ScriptingEngineInternalService.hpp>
#include <editor/scripting/ScriptingGlue.hpp>
#include <robot2D/Util/Logger.hpp>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <iostream>
namespace {
    using namespace editor;
    using namespace tests;
    class Interactor final : public IScriptInteractor {
    public:
        std::vector<SceneEntity> entities;
        SceneEntity getEntity(std::uint64_t id) override {
            for(auto entity: entities) if(id == entity.getUUID()) return entity;
            return {};
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
        robot2D::MessageBus bus; Scene scene(bus);
        auto health = scene.createEntity("Health"); health.addComponent<TextComponent>();
        auto wave = scene.createEntity("Wave"); wave.addComponent<TextComponent>();
        auto status = scene.createEntity("Status"); status.addComponent<TextComponent>();
        mono_set_dirs(RB2D_TEST_MONO_LIB, RB2D_TEST_MONO_ETC);
        auto* domain = mono_jit_init("HudRegression"); CHECK(domain);
        auto* core = mono_domain_assembly_open(domain, RB2D_TEST_SCRIPTCORE); CHECK(core);
        Service service; service.image = mono_assembly_get_image(core);
        auto interactor = std::make_shared<Interactor>(); interactor -> entities = {health, wave, status};
        service.setScriptInteractor(interactor); ScriptGlue::setService(&service);
        ScriptGlue::registerFunctions(); ScriptGlue::registerComponents();
        auto* driver = mono_domain_assembly_open(domain, RB2D_TEST_HUD_DRIVER); CHECK(driver);
        auto* klass = mono_class_from_name(mono_assembly_get_image(driver), "", "HudDriver"); CHECK(klass);
        const auto invoke = [klass](const char* name, int count, void** args) {
            auto* method = mono_class_get_method_from_name(klass, name, count); CHECK(method);
            MonoObject* exception = nullptr; auto* value = mono_runtime_invoke(method, nullptr, args, &exception);
            CHECK(!exception && value); return value;
        };
        const auto integer = [&](const char* name, int count, void** args) {
            return *static_cast<int*>(mono_object_unbox(invoke(name, count, args)));
        };
        std::uint64_t h = health.getUUID(), w = wave.getUUID(), s = status.getUUID();
        void* bind[]{&h, &w, &s}; CHECK(integer("Bind", 3, bind) == 1);
        CHECK(health.getComponent<TextComponent>().getText() == "Health: 100");
        for(int phase = 0; phase < 3; ++phase) {
            void* args[]{&phase}; CHECK(integer("Cycle", 1, args) == 1);
            CHECK(health.getComponent<TextComponent>().getText() ==
                (phase == 0 ? "Health: 25" : phase == 1 ? "Health: 0" : "Health: 100"));
            CHECK(wave.getComponent<TextComponent>().getText() == (phase == 2 ? "Wave: 1" : "Wave: 3"));
            CHECK(status.getComponent<TextComponent>().getText() == (phase == 1 ? "Game Over - press R" : ""));
        }
        const std::string unicode = u8"Здоровье: 75";
        void* text[]{mono_string_new(domain, unicode.c_str())}; CHECK(integer("Set", 1, text) == 1);
        CHECK(health.getComponent<TextComponent>().getText() == unicode);
        auto* returned = reinterpret_cast<MonoString*>(invoke("Read", 0, nullptr));
        char* utf8 = mono_string_to_utf8(returned); CHECK(std::string(utf8) == unicode); mono_free(utf8);
        text[0] = mono_string_new_len(domain, "a\0b", 3); CHECK(integer("Set", 1, text) == 0);
        CHECK(health.getComponent<TextComponent>().getText() == unicode);
        text[0] = nullptr;
        CHECK(integer("Set", 1, text) == 1 && health.getComponent<TextComponent>().getText().empty());
        CHECK(integer("Detached", 0, nullptr) == 1);
        health.removeComponent<TextComponent>(); CHECK(integer("Set", 1, text) == 0);
        ScriptGlue::setService(nullptr); CHECK(integer("Set", 1, text) == 0);
        mono_jit_cleanup(domain);
    }
}
int main() {
    logger::Log::Init();
    try { managed(); std::cout << "HUD managed passed\n"; return 0; }
    catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
