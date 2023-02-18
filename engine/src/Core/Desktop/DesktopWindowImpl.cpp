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

#include "DesktopWindowImpl.hpp"

namespace robot2D::priv {
    DesktopWindowImpl::DesktopWindowImpl():
            m_window(nullptr),
            m_size(800, 600),
            m_name("Robot2D"),
            m_context(),
            m_cursorVisible(true)
    {
        setup();
    }

    DesktopWindowImpl::DesktopWindowImpl(const vec2u& size,
                                         const std::string& name,
                                         WindowContext& context):
            m_window(nullptr),
            m_size(size),
            m_name(name),
            m_context(context),
            m_cursorVisible(true)
    {
        setup();
    }

    DesktopWindowImpl::~DesktopWindowImpl() {}

    bool DesktopWindowImpl::pollEvents(Event& event) {
        //TODO: @a.raag sleep/wait some time ?
        if(m_event_queue.empty()){}

        if(!m_event_queue.empty()) {
            event = m_event_queue.front();
            m_event_queue.pop();
            return true;
        }

        return false;
    }

    void* DesktopWindowImpl::getRaw() {
        return static_cast<void*>(m_window);
    }

    void DesktopWindowImpl::setup() {
        /* Initialize the library */
        if (!glfwInit())
            throw std::runtime_error("Can't Init GLFW3");
#ifdef ROBOT2D_MACOS
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        int opengl_major = 0;
        int opengl_minor = 0;

        switch(m_context.renderApi) {
            case WindowContext::RenderApi::OpenGL3_3: {
                opengl_major = 3;
                opengl_minor = 3;
                break;
            }
            case WindowContext::RenderApi::OpenGL4_5: {
                opengl_major = 4;
                opengl_minor = 5;
                break;
            }
        }

        if(m_context.renderApi == WindowContext::RenderApi::OpenGL3_3 ||
           m_context.renderApi == WindowContext::RenderApi::OpenGL4_5) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }

        GLFWmonitor* primary = m_context.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

        /* Create a windowed mode window and its OpenGL context */
        m_window = glfwCreateWindow(static_cast<int>(m_size.x),
                                    static_cast<int>(m_size.y),
                                    m_name.c_str(), primary, nullptr);
        if (!m_window)
        {
            glfwTerminate();
            exit(1);
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(m_window);
        if(m_context.vsync)
            glfwSwapInterval(1);

        setup_callbacks();

#if defined(ROBOT2D_WINDOWS) || defined(ROBOT2D_LINUX)
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
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
        glfwTerminate();
    }

    void DesktopWindowImpl::display() {
        glfwSwapBuffers(m_window);
        glfwPollEvents();
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

    void DesktopWindowImpl::close_callback(GLFWwindow* wnd) {
        auto window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        Event event{};
        event.type = Event::Closed;
        window -> m_event_queue.push(event);
    }

    void DesktopWindowImpl::key_callback(GLFWwindow* wnd, int key, [[maybe_unused]] int scancode,
                                         int action, [[maybe_unused]] int mods) {
        auto window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));

        Event event{};

        if(action == GLFW_REPEAT || action == GLFW_PRESS)
            event.type = Event::KeyPressed;
        if(action == GLFW_RELEASE)
            event.type = Event::KeyReleased;
        event.key.code = Int2Key(key);
        window -> m_event_queue.push(event);
    }

    void DesktopWindowImpl::cursor_callback(GLFWwindow* wnd, double xpos, double ypos) {
        auto window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));

        Event event{};
        event.move.x = static_cast<float>(xpos);
        event.move.y = static_cast<float>(ypos);
        event.type = Event::MouseMoved;

        window->m_event_queue.push(event);
    }

    void DesktopWindowImpl::mouseWhell_callback(GLFWwindow* wnd, [[maybe_unused]] double xpos, double ypos) {
        auto window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        Event event{};
        event.type = Event::MouseWheel;
        event.wheel.scroll_x = 0;
        event.wheel.scroll_y = static_cast<float>(ypos);
        window->m_event_queue.push(event);
    }

    void DesktopWindowImpl::mouse_callback(GLFWwindow* wnd, int button, int action, [[maybe_unused]] int mods) {
        auto window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));

        double xpos, ypos;
        glfwGetCursorPos(wnd, &xpos, &ypos);

        Event event{};
        event.mouse.btn = button;
        event.mouse.x = static_cast<int>(xpos);
        event.mouse.y = static_cast<int>(ypos);

        if(action == GLFW_PRESS)
            event.type = Event::MousePressed;

        if (action == GLFW_RELEASE)
            event.type = Event::MouseReleased;

        window -> m_event_queue.push(event);
    }

    void DesktopWindowImpl::size_callback(GLFWwindow* wnd, int w, int h) {
        auto window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        Event event{};
        event.type = Event::Resized;
        event.size.widht = w;
        event.size.heigth = h;

        window -> m_event_queue.push(event);
    }

    void DesktopWindowImpl::framebuffer_callback([[maybe_unused]] GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    void DesktopWindowImpl::maximized_callback([[maybe_unused]] GLFWwindow* wnd, [[maybe_unused]] int state) {}

    void DesktopWindowImpl::focus_callback(GLFWwindow* wnd, int focus) {
        auto window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        Event event{};
        if(focus) {
            event.type = Event::GainFocus;
        } else {
            event.type = Event::LostFocus;
        }

        window->m_event_queue.push(event);
    }

    void DesktopWindowImpl::text_callback(GLFWwindow* wnd, unsigned int symbol) {
        auto window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
        Event event{};
        event.type = Event::TextEntered;
        event.text.symbol = symbol;
        window->m_event_queue.push(event);
    }

    void DesktopWindowImpl::dragdrop_callback([[maybe_unused]] GLFWwindow* wnd, int count, const char** paths) {
        std::vector<std::string> outputPaths;
        for(int it = 0; it < count; ++it) {
            if(paths[it] != nullptr)
                outputPaths.emplace_back(paths[it]);
        }
    }

    void DesktopWindowImpl::joystick_callback(int jid, int evt) {
        if(evt == GLFW_CONNECTED) {
            auto window = static_cast<DesktopWindowImpl*>(glfwGetJoystickUserPointer(jid));
            if(!window || jid != GLFW_JOYSTICK_1)
                return;

            Event event{};
            event.type = Event::JoystickConnected;
            event.joystick.joytickId = jid;
            event.joystick.isGamepad = glfwJoystickIsGamepad(jid);
            window -> m_event_queue.push(event);
        }

        if(evt == GLFW_DISCONNECTED) {
            auto window = static_cast<DesktopWindowImpl*>(glfwGetJoystickUserPointer(jid));
            if(!window || jid != GLFW_JOYSTICK_1)
                return;
            Event event{};
            event.type = Event::JoystickDisconnected;
            event.joystick.joytickId = jid;
            window -> m_event_queue.push(event);
        }
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

    bool DesktopWindowImpl::isKeyboardPressed(const Key &key) {
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
        return {static_cast<float>(pos_x),
                static_cast<float>(pos_y)};
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
}