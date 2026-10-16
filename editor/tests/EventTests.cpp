#include "TestSupport.hpp"
#include <editor/EventBinder.hpp>
#include <editor/UISystem.hpp>
#include <editor/Components.hpp>
#include <editor/components/UIHitBox.hpp>
#include <robot2D/Ecs/Scene.hpp>
#include <editor/EventHistory.hpp>
#include <editor/ShortCutManager.hpp>
#include <robot2D/Core/EventQueue.hpp>
#include <robot2D/imgui/InputEvents.hpp>
#include <imgui/imgui.h>
#include <iostream>

namespace {
    using robot2D::Event;
    using editor::InputRoute;

    Event key(bool down, robot2D::Key code = robot2D::Key::A, bool repeat = false) {
        if(down) return Event(Event::KeyPressed{{code, 12, 0}, repeat});
        return Event(Event::KeyReleased{{code, 12, 0}});
    }
    Event button(bool down, float x = 10, float y = 20) {
        if(down) return Event(Event::MousePressed{{0, x, y, 0}});
        return Event(Event::MouseReleased{{0, x, y, 0}});
    }
    void variant() {
        Event empty;
        CHECK(empty.type() == Event::Type::None);
        CHECK(!empty.getIf<Event::KeyPressed>());
        Event input = key(true);
        CHECK(input.get<Event::KeyPressed>().scancode == 12);
        CHECK(input.getIf<Event::KeyboardEvent>());
        CHECK(!input.getIf<Event::MousePressed>());
        bool wrong = false;
        try { input.get<Event::MouseMoved>(); } catch(const std::bad_variant_access&) { wrong = true; }
        CHECK(wrong);
        Event copy;
        { Event original(Event::FilesDropped{{"a/file.png", "b/font.ttf"}}); copy = original; }
        CHECK(copy.get<Event::FilesDropped>().paths[1] == "b/font.ttf");
    }
    struct InputCamera final : editor::IEditorCamera {
        InputCamera() : IEditorCamera(editor::EditorCameraType::Orthographic) {
        }
        void setViewportSize(robot2D::vec2f) override {
        }
        void handleEvents(const Event&) override {
        }
        void update(robot2D::vec2f, float) override {
        }
        robot2D::vec2f convertPixelToCoords(robot2D::vec2f value) const override {
            return value;
        }
    };
    void runtimeClickOrder() {
        robot2D::MessageBus bus;
        robot2D::ecs::Scene scene(bus, false);
        editor::UISystem ui(bus);
        ui.setCamera(std::make_shared<InputCamera>());
        std::vector<int> received;
        const auto callback = ui.addMousePressedCallback([&](robot2D::ecs::Entity entity, std::uint64_t) {
            received.push_back(static_cast<int>(entity.getIndex()));
        });
        const auto add = [&](float x) {
            auto entity = scene.createEntity();
            entity.addComponent<editor::TransformComponent>();
            auto& hitbox = entity.addComponent<editor::UIHitbox>();
            hitbox.m_area = {x, 0, 20, 20};
            hitbox.callbackIDs[editor::UIHitbox::MouseDown] = static_cast<std::int32_t>(callback);
            CHECK(ui.addEntity(entity));
            return entity;
        };
        const auto first = add(0);
        const auto second = add(100);
        ui.handleEvents(button(true, 105, 5));
        ui.handleEvents(button(false, 105, 5));
        ui.handleEvents(button(true, 5, 5));
        ui.handleEvents(button(false, 5, 5));
        ui.handleEvents(Event(Event::MouseMoved{{500, 500}}));
        ui.update(0.016F);
        CHECK(received == std::vector<int>({static_cast<int>(second.getIndex()), static_cast<int>(first.getIndex())}));
        ui.update(0.016F);
        CHECK(received.size() == 2);
        ui.handleEvents(button(true, 5, 5));
        ui.handleEvents(Event(Event::LostFocus{}));
        ui.update(0.016F);
        CHECK(received.size() == 2);
    }
    void order() {
        runtimeClickOrder();
        robot2D::EventQueue queue;
        queue.push(Event::MousePressed{{0, 1.5F, 2.5F, 0}});
        queue.push(Event::MouseMoved{{3.5F, 4.5F}});
        queue.push(Event::MouseReleased{{0, 5.5F, 6.5F, 0}});
        auto batch = queue.take();
        CHECK(batch.size() == 3);
        CHECK(batch[0].type() == Event::Type::MousePressed);
        CHECK(batch[1].type() == Event::Type::MouseMoved);
        CHECK(batch[2].type() == Event::Type::MouseReleased);
        CHECK(batch[0].get<Event::MousePressed>().x == 1.5F);
        CHECK(batch[2].get<Event::MouseReleased>().x == 5.5F);
        CHECK(batch[0].context().sequence + 1 == batch[1].context().sequence);
        CHECK(batch[1].context().sequence + 1 == batch[2].context().sequence);
        CHECK(batch[0].context().receivedAt <= batch[2].context().receivedAt);
        CHECK(queue.take().empty());
        queue.push(Event::Closed{});
        Event out;
        CHECK(queue.poll(out) && out.context().sequence == 4);
        CHECK(!queue.poll(out));
    }
    void binder() {
        editor::EventBinder binder;
        std::vector<int> received;
        binder.bind<Event::MousePressed>([&](const Event::MousePressed& event, const robot2D::EventContext& context) {
            CHECK(context.frame == 7); received.push_back(static_cast<int>(event.x));
            return editor::EventResult::Handled;
        });
        binder.bind<Event::MouseMoved>([&](const Event::MouseMoved& event) {
            received.push_back(static_cast<int>(event.x));
        });
        CHECK(binder.dispatch(button(true, 1), {1, 7, robot2D::EventTime{0}}) == editor::EventResult::Handled);
        CHECK(binder.dispatch(Event(Event::MouseMoved{{2, 0}})) == editor::EventResult::Handled);
        CHECK(binder.dispatch(key(true)) == editor::EventResult::Ignored);
        CHECK(received == std::vector<int>({1, 2}));
        binder.bind<Event::KeyPressed>([](const Event::KeyPressed&) { return editor::EventResult::Ignored; });
        CHECK(binder.dispatch(key(true)) == editor::EventResult::Ignored);
        binder.bind<Event::KeyPressed>([](const Event::KeyPressed&) -> editor::EventResult {
            throw std::runtime_error("command failed");
        });
        bool propagated = false;
        try { binder.dispatch(key(true)); } catch(const std::runtime_error&) { propagated = true; }
        CHECK(propagated);
        binder.bind<Event::KeyPressed>([&](const Event::KeyPressed&) { binder.unbind<Event::KeyPressed>(); });
        bool mutationRejected = false;
        try { binder.dispatch(key(true)); } catch(const std::logic_error&) { mutationRejected = true; }
        CHECK(mutationRejected);
        binder.unbind<Event::KeyPressed>(); // Dispatch scope also unwinds on exceptions.
        binder.unbind<Event::MousePressed>();
        CHECK(binder.dispatch(button(true)) == editor::EventResult::Ignored);
    }
    struct GuiContext {
        GuiContext() {
            ImGui::CreateContext();
            auto& io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.DisplaySize = {640, 480};
            io.DeltaTime = 1.F / 60.F;
            unsigned char* pixels = nullptr; int width = 0, height = 0;
            io.Fonts -> GetTexDataAsRGBA32(&pixels, &width, &height);
        }
        ~GuiContext() {
            ImGui::DestroyContext();
        }
        void frame() {
            ImGui::NewFrame(); ImGui::EndFrame();
        }
    };
    void guiTransitions() {
        GuiContext context;
        auto& io = ImGui::GetIO();
        // Even capture=true must not suppress submission of press or release.
        io.WantCaptureKeyboard = true;
        robot2D::feedGuiEvent(io, key(true));
        robot2D::feedGuiEvent(io, key(false));
        context.frame();
        CHECK(ImGui::IsKeyDown(ImGuiKey_A));
        context.frame();
        CHECK(!ImGui::IsKeyDown(ImGuiKey_A));
        robot2D::feedGuiEvent(io, button(true, 12.5F, 24.5F));
        robot2D::feedGuiEvent(io, button(false, 13.5F, 25.5F));
        context.frame();
        CHECK(io.MouseDown[0]);
        context.frame();
        CHECK(!io.MouseDown[0]);
        robot2D::feedGuiEvent(io, key(true, robot2D::Key::Unknown));
        robot2D::feedGuiEvent(io, Event(Event::TextEntered{{0x0416}}));
        ImGui::NewFrame();
        CHECK(io.InputQueueCharacters.Size == 1 && io.InputQueueCharacters[0] == 0x0416);
        ImGui::EndFrame();
        robot2D::feedGuiEvent(io, key(true)); context.frame();
        robot2D::feedGuiEvent(io, Event(Event::LostFocus{})); context.frame();
        CHECK(!ImGui::IsKeyDown(ImGuiKey_A));
    }
    void guiCapture() {
        GuiContext context;
        char text[64]{};
        const auto draw = [&]() {
            ImGui::SetNextWindowPos({0, 0});
            ImGui::SetNextWindowSize({400, 300});
            ImGui::Begin("Input test");
            ImGui::SetKeyboardFocusHere();
            ImGui::InputText("Name", text, sizeof(text));
            ImGui::End();
        };
        for(int i = 0; i < 3; ++i) {
            ImGui::NewFrame(); draw(); ImGui::EndFrame();
        }
        const auto capture = robot2D::captureGuiInput();
        CHECK(capture.keyboard && capture.activeItem);
        editor::InputRouter router;
        CHECK(router.route(key(true), {capture, true, true, false}).target == InputRoute::Gui);
        robot2D::feedGuiEvent(ImGui::GetIO(), Event(Event::TextEntered{{'x'}}));
        for(int i = 0; i < 2; ++i) {
            ImGui::NewFrame(); draw(); ImGui::EndFrame();
        }
        CHECK(std::string(text) == "x");
    }

    void capture() {
        editor::InputRouter router;
        editor::RoutingContext context{{true, true, true, false}, true, true, false};
        CHECK(router.route(key(true), context).target == InputRoute::Gui);
        context.capture = {};
        CHECK(router.route(key(false), context).target == InputRoute::Gui);
        CHECK(router.route(key(true), context).target == InputRoute::Editor);
        context.capture.keyboard = true; context.capture.activeItem = true;
        CHECK(router.route(key(false), context).target == InputRoute::Editor);
        CHECK(router.route(key(false), context).target == InputRoute::Ignored);
        context.capture = {true, false, false, false};
        CHECK(router.route(button(true), context).target == InputRoute::Editor);
        context.mouseInViewport = false;
        CHECK(router.route(Event(Event::MouseMoved{{500, 500}}), context).target == InputRoute::Editor);
        CHECK(router.route(button(false, 500, 500), context).target == InputRoute::Editor);
        CHECK(router.route(button(true), context).target == InputRoute::Gui);
        context.mouseInViewport = true;
        CHECK(router.route(button(false), context).target == InputRoute::Gui);
        CHECK(router.route(button(true), context).target == InputRoute::Editor);
        CHECK(router.route(Event(Event::LostFocus{}), context).cancelInputs);
        CHECK(router.route(button(false), context).target == InputRoute::Ignored);
        router.route(Event(Event::GainFocus{}), context);
        router.route(button(true), context);
        context.capture.popup = true;
        CHECK(router.route(Event(Event::MouseMoved{{1, 2}}), context).cancelInputs);
        CHECK(router.route(button(false), context).target == InputRoute::Ignored);
        context.capture = {}; context.runtime = true;
        const auto decision = router.route(key(true), context);
        CHECK(decision.cancelInputs && decision.target == InputRoute::Runtime);
        context.runtime = false;
        CHECK(router.route(Event{}, context).cancelInputs);
        context.capture.popup = true;
        CHECK(router.route(Event{}, context).cancelInputs);
        CHECK(router.route(Event(Event::Resized{{100, 200}}), context).target == InputRoute::System);
    }
    void shortcuts() {
        editor::ShortCut shortcut{robot2D::Key::LEFT_CONTROL, robot2D::Key::S};
        int calls = 0;
        shortcut.setCallback([&]() { ++calls; });
        shortcut.handleEvents(key(true, robot2D::Key::LEFT_CONTROL));
        shortcut.handleEvents(key(true, robot2D::Key::S));
        shortcut.handleEvents(key(false, robot2D::Key::S));
        CHECK(calls == 1); // No fixed update occurred between press and release.
        shortcut.handleEvents(key(true, robot2D::Key::S, true));
        CHECK(calls == 1);
        shortcut.handleEvents(Event(Event::LostFocus{}));
        CHECK(!shortcut.isDown());
        shortcut.handleEvents(key(true, robot2D::Key::S));
        CHECK(calls == 1);
    }
    void history() {
        editor::EventHistory history(2);
        robot2D::EventQueue queue;
        queue.push(Event::KeyPressed{{robot2D::Key::A, 0, 0}, false});
        queue.push(Event::KeyReleased{{robot2D::Key::A, 0, 0}});
        queue.push(Event::FilesDropped{{"owned.png"}});
        const auto batch = queue.take();
        for(const auto& event : batch) history.record(event, {InputRoute::Gui, false});
        const auto saved = history.snapshot();
        CHECK(saved.size() == 2 && saved[0].event.context().sequence == 2);
        CHECK(history.overwrittenCount() == 1);
        CHECK(batch.size() == 3); // Eviction never changes the delivery batch.
        history.clear();
        CHECK(saved[1].event.get<Event::FilesDropped>().paths[0] == "owned.png");
        history.setRecording(false); history.record(batch[0], {});
        CHECK(history.events().empty());
        editor::EventHistory disabled(0); disabled.record(batch[0], {});
        CHECK(disabled.events().empty());
        editor::EventHistory bounded(4, sizeof(Event) + 32);
        const Event large(Event::FilesDropped{{std::string(256, 'x')}});
        bounded.record(large, {});
        CHECK(bounded.events().empty() && bounded.oversizedCount() == 1);
        CHECK(large.get<Event::FilesDropped>().paths[0].size() == 256);
    }
}

int main(int argc, char** argv) {
    try {
        if(argc != 2) throw std::runtime_error("Expected event test case");
        const std::string name(argv[1]);
        if(name == "variant") variant();
        else if(name == "order") order();
        else if(name == "binder") binder();
        else if(name == "gui") guiTransitions();
        else if(name == "capture") capture();
        else if(name == "gui-capture") guiCapture();
        else if(name == "shortcuts") shortcuts();
        else if(name == "history") history();
        else throw std::runtime_error("Unknown event test case");
        return 0;
    }
    catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
