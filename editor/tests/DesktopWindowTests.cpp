#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <robot2D/Graphics/GL.hpp>
#include <GLFW/glfw3.h>
#include <robot2D/Engine.hpp>
#include <editor/Application.hpp>
#include <robot2D/Core/Keyboard.hpp>
#include <robot2D/Core/Mouse.hpp>
#include "../../core/src/Core/Desktop/DesktopWindowImpl.hpp"

namespace {
    using namespace robot2D;
    using Backend = priv::DesktopWindowImpl;
    static_assert(!std::is_copy_constructible_v<Window>);
    static_assert(!std::is_move_constructible_v<Window>);
    static_assert(std::has_virtual_destructor_v<Window>);
    static_assert(std::is_nothrow_destructible_v<DesktopRuntime>);
    static_assert(std::is_nothrow_destructible_v<Window>);

    void require(bool value, const char* message) {
        if(!value) throw std::runtime_error(message);
    }

    template<typename Action>
    void creationFails(WindowCreationStage stage, Action&& action) {
        try { action(); }
        catch(const WindowCreationError& error) {
            require(error.stage() == stage, "wrong creation failure stage");
            if(stage == WindowCreationStage::Initialization || stage == WindowCreationStage::NativeWindow)
                require(error.nativeErrorCode() != 0, "native error code was lost");
            require(std::string(error.what()).size() > 8, "missing error description");
            return;
        }
        throw std::runtime_error("creation unexpectedly succeeded");
    }

    void uninitialized() {
        creationFails(WindowCreationStage::Initialization, []() {
            DesktopRuntime borrowed(DesktopRuntime::Ownership::Borrowed);
        });
        Cursor empty;
        require(empty.getRaw() == nullptr, "empty cursor allocated a native handle");
        bool rejected = false;
        try { empty.createDefault(); }
        catch(const std::logic_error&) { rejected = true; }
        require(rejected, "cursor allocated without a runtime");
    }

    void editorShutdown() {
        editor::Application application;
        application.destroy();
        application.destroy();
        require(ImGui::GetCurrentContext() == nullptr, "shutdown before setup created a GUI context");
    }

    bool graphicsAvailable() {
        if(!glfwInit()) return false;
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        auto* probe = glfwCreateWindow(32, 32, "availability probe", nullptr, nullptr);
        const bool available = probe != nullptr;
        if(probe) glfwDestroyWindow(probe);
        glfwTerminate();
        return available;
    }

    PFNGLDELETEBUFFERSPROC originalDeleteBuffers = nullptr;
    GLFWwindow* expectedDeleteContext = nullptr;
    int deletedBuffers = 0;
    bool correctDeleteContext = true;
    void APIENTRY observeDeleteBuffers(GLsizei count, const GLuint* buffers) {
        deletedBuffers += count;
        correctDeleteContext &= glfwGetCurrentContext() == expectedDeleteContext;
        originalDeleteBuffers(count, buffers);
    }

    void renderLifetime() {
        DesktopRuntime desktop;
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        auto target = std::make_unique<RenderWindow>(desktop, vec2u{64, 64}, "renderer");
        expectedDeleteContext = static_cast<GLFWwindow*>(target -> getRaw());
        Window other(desktop, {32, 32}, "other context");
        require(glfwGetCurrentContext() == other.getRaw(), "other context was not selected");
        originalDeleteBuffers = glad_glDeleteBuffers;
        glad_glDeleteBuffers = observeDeleteBuffers;
        target.reset();
        glad_glDeleteBuffers = originalDeleteBuffers;
        require(deletedBuffers > 0 && correctDeleteContext, "renderer released GPU buffers with the wrong context");
        other.makeCurrent();
        require(other.isOpen(), "renderer teardown destroyed a different window");
    }

    void lifetime() {
        for(int iteration = 0; iteration != 3; ++iteration) {
            DesktopRuntime desktop;
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            {
                Window first(desktop, {64, 64}, "first");
                const auto firstHandle = first.getRaw();
                const auto firstSize = first.getLogicalSize();
                {
                    Window second(desktop, {96, 64}, "second");
                    require(first.getRaw() == firstHandle && first.getRaw() != second.getRaw(),
                        "creating another window replaced the first backend");
                    first.close();
                    require(!first.isOpen() && second.isOpen(), "close affected another window");
                    require(first.getRaw() == firstHandle, "close destroyed native window");
                    first.cancelClose();
                    require(first.isOpen(), "cancelClose failed");
                }
                first.makeCurrent();
                require(glfwGetCurrentContext() == first.getRaw(), "surviving window lost its context");
                desktop.processEvents();
                require(first.getLogicalSize() == firstSize, "surviving window is invalid");
                Cursor cursor(desktop);
                cursor.createDefault();
                require(cursor.getRaw() != nullptr, "cursor creation failed");
                first.setCursor(cursor);
                Cursor defaultCursor;
                first.setCursor(defaultCursor);
            }
            require(glfwGetCurrentContext() == nullptr, "window destructor did not destroy current context");
        }
    }

    int created = 0;
    int destroyed = 0;
    GLFWwindow* trackedCreate(int width, int height, const char* name, GLFWmonitor* monitor, GLFWwindow* share) {
        auto* result = glfwCreateWindow(width, height, name, monitor, share);
        if(result) ++created;
        return result;
    }
    void trackedDestroy(GLFWwindow* window) {
        ++destroyed; glfwDestroyWindow(window);
    }
    bool failLoader() {
        // Model GLAD returning failure after mutating its global table.
        glad_glGetString = nullptr;
        GLVersion.major = 0;
        return false;
    }
    GLFWwindow* failCreate(int width, int height, const char* name, GLFWmonitor* monitor, GLFWwindow* share) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 99);
        return glfwCreateWindow(width, height, name, monitor, share);
    }

    void rollback() {
        DesktopRuntime desktop;
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        Window first(desktop, {64, 64}, "survivor");
        auto* previous = glfwGetCurrentContext();
        WindowContext context;
        for(const auto size : {vec2u{0, 64}, vec2u{64, 0},
                               vec2u{std::numeric_limits<unsigned>::max(), 64}})
            creationFails(WindowCreationStage::Validation, [&]() { Window invalid(desktop, size, "invalid"); });
        WindowContext invalidContext;
        invalidContext.renderApi = static_cast<WindowContext::RenderApi>(-1);
        creationFails(WindowCreationStage::Validation, [&]() {
            Window invalid(desktop, {64, 64}, "invalid API", invalidContext);
        });
        creationFails(WindowCreationStage::NativeWindow, [&]() {
            Backend failed(desktop, {64, 64}, "native failure", context,
            Backend::NativeFunctions{failCreate, trackedDestroy, nullptr});
        });
        require(destroyed == 0 && glfwGetCurrentContext() == previous, "native failure damaged survivor");
        creationFails(WindowCreationStage::ContextLoader, [&]() {
            Backend failed(desktop, {64, 64}, "loader failure", context,
            Backend::NativeFunctions{trackedCreate, trackedDestroy, failLoader});
        });
        require(created == 1 && destroyed == 1, "partially constructed native window leaked");
        require(glfwGetCurrentContext() == previous, "loader failure did not restore previous context");
        require(glad_glGetString && GLVersion.major >= 4, "loader failure corrupted the survivor GL table");
        require(glGetString(GL_VERSION) != nullptr, "surviving GL context is unusable");
        glfwMakeContextCurrent(nullptr);
        creationFails(WindowCreationStage::ContextLoader, [&]() {
            Backend failed(desktop, {64, 64}, "failure with unbound survivor", context,
            Backend::NativeFunctions{trackedCreate, trackedDestroy, failLoader});
        });
        require(created == 2 && destroyed == 2, "unbound failure leaked a window");
        require(glfwGetCurrentContext() == nullptr, "rollback selected a context that was not current before");
        require(glad_glGetString && GLVersion.major >= 4, "unbound survivor GL table was not restored");
        first.makeCurrent();
        require(glGetString(GL_VERSION) != nullptr, "unbound survivor became unusable");
        {
            Backend success(desktop, {64, 64}, "retry", context,
                Backend::NativeFunctions{trackedCreate, trackedDestroy, nullptr});
        }
        require(created == 3 && destroyed == 3, "normal destruction leaked or double-freed a window");
        first.makeCurrent();
        require(first.isOpen(), "creation failure closed existing window");
        desktop.processEvents();
    }

    int externalJoystickCalls = 0;
    void externalJoystick(int, int) {
        ++externalJoystickCalls;
    }

    void borrowed() {
        require(glfwInit() == GLFW_TRUE, "external GLFW init failed");
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        auto* foreign = glfwCreateWindow(32, 32, "foreign", nullptr, nullptr);
        require(foreign != nullptr, "foreign window unavailable");
        glfwSetJoystickCallback(externalJoystick);
        {
            DesktopRuntime desktop(DesktopRuntime::Ownership::Borrowed);
            bool rejected = false;
            try { DesktopRuntime duplicate; }
            catch(const std::logic_error&) { rejected = true; }
            require(rejected, "duplicate runtime silently took global ownership");
            Window window(desktop, {32, 32}, "borrower");
            bool rejectedThread = false;
            std::thread worker([&]() {
                try { desktop.processEvents(); }
                catch(const std::logic_error&) { rejectedThread = true; }
            });
            worker.join();
            require(rejectedThread, "event processing accepted another thread");
        }
        auto restored = glfwSetJoystickCallback(nullptr);
        require(restored == externalJoystick, "external joystick callback was not restored");
        restored(3, GLFW_CONNECTED);
        require(externalJoystickCalls == 1, "external callback unusable after shutdown");
        require(glfwGetWindowAttrib(foreign, GLFW_VISIBLE) == GLFW_FALSE, "borrower terminated foreign GLFW");
        glfwDestroyWindow(foreign);
        glfwTerminate();
    }

    void events() {
        DesktopRuntime desktop;
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        Window first(desktop, {64, 64}, "first");
        Window second(desktop, {64, 64}, "second");
        auto* one = static_cast<GLFWwindow*>(first.getRaw());
        auto* two = static_cast<GLFWwindow*>(second.getRaw());
        auto keyOne = glfwSetKeyCallback(one, nullptr);
        auto keyTwo = glfwSetKeyCallback(two, nullptr);
        glfwSetKeyCallback(one, keyOne); glfwSetKeyCallback(two, keyTwo);
        first.takeEvents(1); second.takeEvents(1);
        keyOne(one, GLFW_KEY_A, 10, GLFW_PRESS, 0);
        keyTwo(two, GLFW_KEY_B, 11, GLFW_PRESS, 0);
        keyOne(one, GLFW_KEY_A, 10, GLFW_RELEASE, 0);
        auto batch = first.takeEvents(2);
        require(batch.events.size() == 2, "short press/release was lost or queues mixed");
        require(batch.events[0].getIf<Event::KeyPressed>() && batch.events[1].getIf<Event::KeyReleased>(),
            "press/release order was changed");
        auto other = second.takeEvents(2);
        require(other.events.size() == 1 && other.events[0].getIf<Event::KeyPressed>() -> code == Key::B,
            "second window received the wrong event");
        require(!Keyboard::isKeyPressed(first, static_cast<Key>(-1)), "invalid key was polled");
        require(!isMousePressed(second, static_cast<Mouse>(-1)), "invalid mouse button was polled");
        require(!desktop.isJoystickAvailable(static_cast<JoystickType>(-1)), "invalid joystick was polled");
        auto joystick = glfwSetJoystickCallback(nullptr);
        glfwSetJoystickCallback(joystick);
        joystick(GLFW_JOYSTICK_4, GLFW_CONNECTED);
        joystick(GLFW_JOYSTICK_4, GLFW_DISCONNECTED);
        batch = first.takeEvents(3);
        other = second.takeEvents(3);
        require(batch.events.size() == 2 && other.events.size() == 2, "global joystick delivery lost events");
        require(batch.events[0].getIf<Event::JoystickConnected>() != nullptr &&
            batch.events[1].getIf<Event::JoystickDisconnected>() != nullptr, "joystick order changed");
        first.makeCurrent();
        std::array<GLint, 4> before{}, after{};
        glViewport(1, 2, 31, 29); glGetIntegerv(GL_VIEWPORT, before.data());
        auto resize = glfwSetFramebufferSizeCallback(two, nullptr);
        glfwSetFramebufferSizeCallback(two, resize);
        resize(two, 128, 128);
        glGetIntegerv(GL_VIEWPORT, after.data());
        require(before == after, "native callback modified an unrelated current context");
    }

    struct EngineResult { int cleanup = 0; int destructors = 0; bool validContext = true; };
    class ProbeApplication final : public Application {
    public:
        ProbeApplication(EngineResult& result, int failure) : m_result(result), m_failure(failure) {
        }
        ~ProbeApplication() override {
            ++m_result.destructors;
            m_result.validContext &= m_window && glfwGetCurrentContext() == m_window -> getRaw();
            if(m_foreign) glfwDestroyWindow(m_foreign);
        }
        void setup() override {
            if(m_failure == 1) throw std::runtime_error("setup failure");
            if(m_failure >= 5) {
                m_foreign = glfwCreateWindow(32, 32, "cleanup context", nullptr, nullptr);
                require(m_foreign != nullptr, "cleanup context unavailable");
            }
        }
        void guiUpdate(float) override {
            if(m_failure == 2 || m_failure == 3 || m_failure == 6) throw std::runtime_error("frame failure");
            m_running = false;
        }
        void render() override {
        }
        void destroy() override {
            ++m_result.cleanup;
            m_result.validContext &= m_window && glfwGetCurrentContext() == m_window -> getRaw();
            if(m_foreign) glfwMakeContextCurrent(m_foreign);
            if(m_failure == 3 || m_failure == 4 || m_failure == 6) throw std::runtime_error("cleanup failure");
        }
    private:
        EngineResult& m_result;
        int m_failure;
        GLFWwindow* m_foreign = nullptr;
    };

    void engine() {
        for(int failure = 0; failure <= 6; ++failure) {
            EngineResult result;
            Engine host;
            EngineConfiguration config({64, 64}, "engine teardown");
            bool threw = false;
            try { host.run(std::make_unique<ProbeApplication>(result, failure), std::move(config)); }
            catch(const std::runtime_error& error) {
                threw = true;
                const std::string expected = failure == 1 ? "setup failure" :
                    (failure == 4 ? "cleanup failure" : "frame failure");
                require(error.what() == expected, "shutdown replaced the original failure");
            }
            require(threw == (failure != 0 && failure != 5), "wrong Engine error result");
            require(result.cleanup == 1 && result.destructors == 1 && result.validContext,
                "Engine failed to clean up exactly once with the window context current");
            require(glfwGetCurrentContext() == nullptr, "Engine retained its native window");
        }
    }
}

int main(int argc, char** argv) {
    try {
        if(argc != 2) throw std::runtime_error("expected scenario");
        const std::string scenario = argv[1];
        if(scenario == "uninitialized") uninitialized();
        else if(scenario == "editor-shutdown") editorShutdown();
        else {
            if(!graphicsAvailable()) return 77;
            if(scenario == "lifetime") lifetime();
            else if(scenario == "render-lifetime") renderLifetime();
            else if(scenario == "rollback") rollback();
            else if(scenario == "borrowed") borrowed();
            else if(scenario == "events") events();
            else if(scenario == "engine") engine();
            else throw std::runtime_error("unknown scenario");
        }
        std::cout << scenario << " passed\n";
        return 0;
    }
    catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
