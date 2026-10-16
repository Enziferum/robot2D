/*********************************************************************
   (c) Alex Raag 2023
   https://github.com/Enziferum
   robot2D - Zlib license.
   This software is provided 'as-is', without any express or
   implied warranty. In no event will the authors be held
   liable for any damages arising from the use of this software.
   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute
   it freely, subject to the following restrictions:
   1. The origin of this software must not be misrepresented;
   you must not claim that you wrote the original software.
   If you use this software in a product, an acknowledgment
   in the product documentation would be appreciated but
   is not required.
   2. Altered source versions must be plainly marked as such,
   and must not be misrepresented as being the original software.
   3. This notice may not be removed or altered from any
   source distribution.
*********************************************************************/

#include <robot2D/Util/Logger.hpp>
#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Config.hpp>
#include "DesktopWindowImpl.hpp"

namespace robot2D::priv {

    DesktopWindowImpl::DesktopWindowImpl() :
        m_window(nullptr),
        m_size(800, 600),
        m_name("Robot2D"),
        m_context(),
        m_cursorVisible(true) {
        setup();
    }

    DesktopWindowImpl::DesktopWindowImpl(const vec2u& size,
        const std::string& name,
        WindowContext& context) :
        m_window(nullptr),
        m_size(size),
        m_name(name),
        m_context(context),
        m_cursorVisible(true) {
        setup();
    }

    DesktopWindowImpl::~DesktopWindowImpl() = default;

    bool DesktopWindowImpl::pollEvents(Event& event) {
        return m_event_queue.poll(event);
    }

    std::vector<Event> DesktopWindowImpl::takeEvents() {
        return m_event_queue.take();
    }

    void DesktopWindowImpl::processEvents() {
        glfwPollEvents();
        if(m_callbackError) {
            auto error = std::exchange(m_callbackError, {});
            std::rethrow_exception(error);
        }
    }

    void* DesktopWindowImpl::getRaw() {
        return static_cast<void*>(m_window);
    }

    void DesktopWindowImpl::setup() {
        /* Initialize the library */
        if(!glfwInit())
            throw std::runtime_error("Can't Init GLFW3");

        RB_CORE_INFO("Engine's Version {0}, build {1}", RB2_VERSION_FULL, RB2_BUILD_ID);
        RB_CORE_WARN("Window background is GLFW3 with flags: {0}", glfwGetVersionString());

#ifdef ROBOT2D_MACOS
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        int opengl_major = 0;
        int opengl_minor = 0;

        switch(m_context.renderApi) {
        case WindowContext::RenderApi::OpenGL4_3: {
            opengl_major = 4;
            opengl_minor = 3;
            break;
        }
        case WindowContext::RenderApi::OpenGL4_5: {
            opengl_major = 4;
            opengl_minor = 5;
            break;
        }
        }
        RB_CORE_WARN("Engine's GraphicsApi is OpenGL, Version {0}.{1}.", std::to_string(opengl_major),
            std::to_string(opengl_minor));

        if(m_context.renderApi == WindowContext::RenderApi::OpenGL4_3 ||
            m_context.renderApi == WindowContext::RenderApi::OpenGL4_5) {
#if !defined(NDEBUG)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }

        GLFWmonitor* primary = m_context.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

        /* Create a windowed mode window and its OpenGL context */
        m_window = glfwCreateWindow(static_cast<int>(m_size.x),
                static_cast<int>(m_size.y),
                m_name.c_str(), primary, nullptr);
        if(!m_window) {
            glfwTerminate();
            exit(1);
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(m_window);
        if(m_context.vsync)
            glfwSwapInterval(1);

        setup_callbacks();

#if defined(ROBOT2D_WINDOWS) || defined(ROBOT2D_LINUX)
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            RB_CORE_CRITICAL("Failed to initialize GLAD");
            throw std::runtime_error("Failed to initialize GLAD");
        }
#endif
        glViewport(0, 0, static_cast<int>(m_size.x), static_cast<int>(m_size.y));
    }

    bool DesktopWindowImpl::isOpen() const {
        return !glfwWindowShouldClose(m_window);
    }

    void DesktopWindowImpl::setTitle(const std::string& title) const {
        const char* c_str = title.c_str();
        glfwSetWindowTitle(m_window, c_str);
    }

    void DesktopWindowImpl::close() {
        glfwSetWindowShouldClose(m_window, true);
    }

    void DesktopWindowImpl::display() {
        glfwSwapBuffers(m_window);
    }

    void DesktopWindowImpl::setup_callbacks() {
        if(m_window == nullptr)
            throw std::runtime_error("Window is nullptr");

        glfwSetWindowUserPointer(m_window, this);
        glfwSetWindowCloseCallback(m_window, close_callback);
        glfwSetKeyCallback(m_window, key_callback);
        glfwSetCursorPosCallback(m_window, cursor_callback);
        glfwSetScrollCallback(m_window, mouseWhell_callback);
        glfwSetMouseButtonCallback(m_window, mouse_callback);
        glfwSetFramebufferSizeCallback(m_window, framebuffer_callback);
        glfwSetWindowSizeCallback(m_window, size_callback);
        glfwSetWindowMaximizeCallback(m_window, maximized_callback);
        glfwSetDropCallback(m_window, dragdrop_callback);
        glfwSetWindowFocusCallback(m_window, focus_callback);
        glfwSetCharCallback(m_window, text_callback);
        glfwSetJoystickUserPointer(GLFW_JOYSTICK_1, this);
        glfwSetJoystickCallback(joystick_callback);
    }

    void DesktopWindowImpl::cancelClose() {
        glfwSetWindowShouldClose(m_window, false);
    }

    void DesktopWindowImpl::close_callback(GLFWwindow* wnd) {
        auto* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        // A request is delivered before Engine decides whether to stop.

        if(window) window -> enqueue(Event::Closed{});
    }

    void DesktopWindowImpl::key_callback(GLFWwindow* wnd, int key, int scancode, int action, int mods) {
        auto* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        if(!window) return;
        const Event::KeyboardEvent data{Int2Key(key), scancode, static_cast<unsigned>(mods)};
        if(action == GLFW_PRESS || action == GLFW_REPEAT)
            window -> enqueue(Event::KeyPressed{data, action == GLFW_REPEAT});
        else if(action == GLFW_RELEASE)
            window -> enqueue(Event::KeyReleased{data});
    }

    void DesktopWindowImpl::cursor_callback(GLFWwindow* wnd, double xpos, double ypos) {
        auto* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        if(window) window -> enqueue(Event::MouseMoved{{static_cast<float>(xpos), static_cast<float>(ypos)}});
    }

    void DesktopWindowImpl::mouseWhell_callback(GLFWwindow* wnd, double xpos, double ypos) {
        auto* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        if(!window) return;
        double x = 0, y = 0;
        glfwGetCursorPos(wnd, &x, &y);
        window -> enqueue(Event::MouseWheel{{static_cast<float>(xpos), static_cast<float>(ypos),
                static_cast<float>(x), static_cast<float>(y)}});
    }

    void DesktopWindowImpl::mouse_callback(GLFWwindow* wnd, int button, int action, int mods) {
        auto* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        if(!window) return;
        double x = 0, y = 0;
        glfwGetCursorPos(wnd, &x, &y);
        const Event::MouseButtonEvent data{button, static_cast<float>(x), static_cast<float>(y),
                                           static_cast<unsigned>(mods)};
        if(action == GLFW_PRESS) window -> enqueue(Event::MousePressed{data});
        else if(action == GLFW_RELEASE) window -> enqueue(Event::MouseReleased{data});
    }

    void DesktopWindowImpl::size_callback(GLFWwindow* wnd, int w, int h) {
        auto* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        if(window) window -> enqueue(Event::Resized{{static_cast<unsigned>(w), static_cast<unsigned>(h)}});
    }

    void DesktopWindowImpl::framebuffer_callback([[maybe_unused]] GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    void DesktopWindowImpl::maximized_callback([[maybe_unused]] GLFWwindow* wnd, [[maybe_unused]] int state) {
    }

    void DesktopWindowImpl::focus_callback(GLFWwindow* wnd, int focus) {
        auto* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        if(!window) return;
        if(focus) window -> enqueue(Event::GainFocus{});
        else window -> enqueue(Event::LostFocus{});
    }

    void DesktopWindowImpl::text_callback(GLFWwindow* wnd, unsigned int symbol) {
        auto* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        if(window) window -> enqueue(Event::TextEntered{{symbol}});
    }

    void DesktopWindowImpl::dragdrop_callback(GLFWwindow* wnd, int count, const char** paths) {
        auto* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        if(!window) return;
        try {
            Event::FilesDropped data;
            for(int i = 0; i < count; ++i)
                if(paths[i]) data.paths.emplace_back(paths[i]);
            window -> enqueue(std::move(data));
        }
        catch(...) { window -> m_callbackError = std::current_exception(); }
    }

    void DesktopWindowImpl::joystick_callback(int jid, int action) {
        auto* window = static_cast<DesktopWindowImpl*>(glfwGetJoystickUserPointer(jid));
        if(!window || jid != GLFW_JOYSTICK_1) return;
        if(action == GLFW_CONNECTED)
            window -> enqueue(Event::JoystickConnected{{jid, glfwJoystickIsGamepad(jid) == GLFW_TRUE}});
        else if(action == GLFW_DISCONNECTED)
            window -> enqueue(Event::JoystickDisconnected{{jid, false}});
    }

    void DesktopWindowImpl::setIcon(Image&& icon) {
        GLFWimage img;
        auto size = icon.getSize();
        img.width = size.x;
        img.height = size.y;
        img.pixels = icon.getBuffer();
        glfwSetWindowIcon(m_window, 1, &img);
    }

    bool DesktopWindowImpl::isMousePressed(const Mouse& button) {
        return (glfwGetMouseButton(m_window, static_cast<int>(button)) == GLFW_PRESS);
    }

    bool DesktopWindowImpl::isKeyboardPressed(const Key& key) {
        return glfwGetKey(m_window, key2Int(key));
    }

    bool DesktopWindowImpl::isJoystickAvailable(const JoystickType& joystickType) {
        return (glfwJoystickPresent(static_cast<int>(joystickType)) == GLFW_TRUE);
    }

    bool DesktopWindowImpl::JoystickIsGamepad(const JoystickType& joystickType) {
        return (glfwJoystickIsGamepad(static_cast<int>(joystickType)) == GLFW_TRUE);
    }

    void DesktopWindowImpl::setMouseCursorVisible(const bool& flag) {
        m_cursorVisible = flag;
        glfwSetInputMode(m_window, GLFW_CURSOR, (m_cursorVisible ? GLFW_CURSOR_NORMAL: GLFW_CURSOR_HIDDEN));
    }

    void DesktopWindowImpl::setCursor(const Cursor& cursor) {
        auto rawCursor = (GLFWcursor*)cursor.getRaw();
        glfwSetCursor(m_window, rawCursor);
    }

    robot2D::vec2u DesktopWindowImpl::getMonitorSize() const {
        auto videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        if(videoMode == nullptr)
            return {};

        return {static_cast<unsigned int>(videoMode->width),
                static_cast<unsigned int>(videoMode->height)};
    }

    void DesktopWindowImpl::setPosition(const vec2u& position) {
        glfwSetWindowPos(m_window, static_cast<int>(position.x), static_cast<int>(position.y));
    }

    robot2D::vec2u DesktopWindowImpl::getPosition() const {
        robot2D::vec2i pos;
        glfwGetWindowPos(m_window, &pos.x, &pos.y);
        return robot2D::vec2u{static_cast<unsigned int>(pos.x),
                              static_cast<unsigned int>(pos.y)};
    }

    void DesktopWindowImpl::setSize(const vec2u& size) {
        glfwSetWindowSize(m_window, static_cast<int>(size.x), static_cast<int>(size.y));
    }

    void DesktopWindowImpl::setResizable(const bool& flag) {
        glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, flag ? GLFW_TRUE : GLFW_FALSE);
    }

    void DesktopWindowImpl::setMaximazed([[maybe_unused]] const bool& flag) {
        glfwMaximizeWindow(m_window);
    }

    void DesktopWindowImpl::setMousePos(const vec2f& pos) {
        glfwSetCursorPos(m_window, pos.x, pos.y);
    }

    vec2f DesktopWindowImpl::getMousePos() const {
        double pos_x, pos_y;
        glfwGetCursorPos(m_window, &pos_x, &pos_y);
        return {
            static_cast<float>(pos_x),
            static_cast<float>(pos_y)
        };
    }

    bool DesktopWindowImpl::getJoystickGamepadInput(const JoystickType& joystickType,
        JoystickGamepadInput& gamepadInput) {
        GLFWgamepadstate wgamepadstate;
        auto status = glfwGetGamepadState(static_cast<int>(joystickType), &wgamepadstate);
        if(status != GLFW_TRUE)
            return false;

        // TODO(a.raag): remove magic constraints //

        gamepadInput.leftManipulatorOffset = {
            wgamepadstate.axes[GLFW_GAMEPAD_AXIS_LEFT_X],
            wgamepadstate.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]
        };

        if(gamepadInput.leftManipulatorOffset.x == 1.5259022e-05f) {
            gamepadInput.leftManipulatorOffset.x = 0;
        }

        if(gamepadInput.leftManipulatorOffset.y == -1.5259022e-05f) {
            gamepadInput.leftManipulatorOffset.y = 0;
        }

        gamepadInput.rightManipulatorOffset = {
            wgamepadstate.axes[GLFW_GAMEPAD_AXIS_RIGHT_X],
            wgamepadstate.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]
        };

        if(gamepadInput.rightManipulatorOffset.x == 1.5259022e-05f) {
            gamepadInput.rightManipulatorOffset.x = 0;
        }

        if(gamepadInput.rightManipulatorOffset.y == -1.5259022e-05f) {
            gamepadInput.rightManipulatorOffset.y = 0;
        }

        gamepadInput.leftTrigger = wgamepadstate.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
        gamepadInput.rightTrigger = wgamepadstate.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];

        for(int i = 0; i < GLFW_GAMEPAD_BUTTON_LAST + 1; ++i)
            gamepadInput.buttons[i] = static_cast<bool>(wgamepadstate.buttons[i]);

        return true;
    }

    void DesktopWindowImpl::setVsync(bool flag) {
        glfwSwapInterval(flag ? 1 : 0);
    }

    void DesktopWindowImpl::addDropCallback(std::function<void(std::vector<std::string>&&)>&& callback) {
        m_dropCallback = std::move(callback);
    }
}
