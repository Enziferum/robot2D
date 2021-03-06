/*********************************************************************
(c) Alex Raag 2021
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

namespace robot2D {
    namespace priv {

        constexpr int opengl_major = 3;
        constexpr int opengl_minor = 3;

        DesktopWindowImpl::DesktopWindowImpl():
        m_window(nullptr),
        m_size(800, 600),
        m_name("Robot2D"),
        m_context(),
        m_cursorVisible(true)
        {
            setup();
        }

        DesktopWindowImpl::DesktopWindowImpl(const vec2u &size, const std::string &name,
                                             WindowContext &context):
                m_window(nullptr),
                m_size(size),
                m_name(name),
                m_context(context),
                m_cursorVisible(true){
            setup();
        }

        DesktopWindowImpl::~DesktopWindowImpl() {
            glfwTerminate();
        }

        bool DesktopWindowImpl::pollEvents(Event& event) {
            if(m_event_queue.empty()){
                //what todo ??
            }

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
                return;

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

            GLFWmonitor* primary = m_context.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

            /* Create a windowed mode window and its OpenGL context */
            m_window = glfwCreateWindow(m_size.x, m_size.y,
                                        m_name.c_str(), primary, nullptr);
            if (!m_window)
            {
                //todo throw expeption, after termimate
                glfwTerminate();
                exit(1);
            }

            /* Make the window's context current */
            glfwMakeContextCurrent(m_window);
            if(m_context.vsync)
                glfwSwapInterval(1);

            setup_callbacks();
            setup_WGL();
        }

        void DesktopWindowImpl::setup_WGL() {
#ifdef WIN32
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            {
                LOG_ERROR_E("Failed to initialize GLAD")
                return ;
            }
#endif
            glViewport(0, 0, m_size.x, m_size.y);
        }

        bool DesktopWindowImpl::isOpen() const {
            return !glfwWindowShouldClose(m_window);
        }

        void DesktopWindowImpl::setTitle(const std::string& title) const {
            const char* t = title.c_str();
            glfwSetWindowTitle(m_window, t);
        }

        void DesktopWindowImpl::clear(const Color &color) {
            glClearColor(color.r, color.g, color.b, color.alpha);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void DesktopWindowImpl::close() {
            glfwSetWindowShouldClose(m_window, true);
        }

        void DesktopWindowImpl::display() {
            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }

        void DesktopWindowImpl::setup_callbacks() {
            if(m_window == nullptr)
                return;

            glfwSetWindowUserPointer(m_window, this);
            glfwSetWindowCloseCallback(m_window, close_callback);
            glfwSetKeyCallback(m_window, key_callback);
            glfwSetCursorPosCallback(m_window, cursor_callback);
            glfwSetScrollCallback(m_window, mouseWhell_callback);
            glfwSetMouseButtonCallback(m_window, mouse_callback);
            glfwSetFramebufferSizeCallback(m_window, view_callback);
            glfwSetWindowSizeCallback(m_window, size_callback);
            glfwSetWindowMaximizeCallback(m_window, maximized_callback);
            glfwSetDropCallback(m_window, dragdrop_callback);
            glfwSetWindowFocusCallback(m_window, focus_callback);
            glfwSetCharCallback(m_window, text_callback);
        }

        void DesktopWindowImpl::close_callback(GLFWwindow* wnd) {
            DesktopWindowImpl* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
            Event event;
            event.type = Event::Closed;
            window->m_event_queue.push(event);
        }

        void DesktopWindowImpl::key_callback(GLFWwindow* wnd, int key, int scancode, int action, int mods) {
            //c++ hack before c++ 17 to set unused parameter
            (void)(scancode);
            //c++ hack before c++ 17 to set unused parameter
            (void)(mods);

            DesktopWindowImpl* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));

            Event event;
            if(action == GLFW_REPEAT || action == GLFW_PRESS)
                event.type = Event::KeyPressed;
            if(action == GLFW_RELEASE)
                event.type = Event::KeyReleased;
            event.key.code = Int2Key(key);
            window->m_event_queue.push(event);
        }

        void DesktopWindowImpl::cursor_callback(GLFWwindow* wnd, double xpos, double ypos) {
            DesktopWindowImpl* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));

            Event event;
            event.move.x = float(xpos);
            event.move.y = float(ypos);
            event.type = Event::MouseMoved;

            window->m_event_queue.push(event);
        }

        void DesktopWindowImpl::mouseWhell_callback(GLFWwindow* wnd, double xpos, double ypos) {
            //c++ hack before c++ 17 to set unused parameter
            (void)(xpos);

            DesktopWindowImpl* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
            Event event;
            event.type = Event::MouseWheel;
            event.wheel.scroll_x = 0;
            event.wheel.scroll_y += float(ypos);
            window->m_event_queue.push(event);
        }

        void DesktopWindowImpl::mouse_callback(GLFWwindow* wnd, int button, int action, int mods) {
            //c++ hack before c++ 17 to set unused parameter
            (void)(mods);

            DesktopWindowImpl* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));

            Event event;
            event.mouse.btn = button;
            double xpos, ypos;
            glfwGetCursorPos(wnd, &xpos, &ypos);
            event.mouse.x = int(xpos);
            event.mouse.y = int(ypos);

            if(action == GLFW_PRESS)
                event.type = Event::MousePressed;

            if (action == GLFW_RELEASE)
                event.type = Event::MouseReleased;

            window->m_event_queue.push(event);
        }

        void DesktopWindowImpl::size_callback(GLFWwindow* wnd, int w, int h) {
            //c++ hack before c++ 17 to set unused parameter
            (void)(wnd);
            //c++ hack before c++ 17 to set unused parameter
            (void)(w);
            //c++ hack before c++ 17 to set unused parameter
            (void)(h);
        }

        void DesktopWindowImpl::view_callback(GLFWwindow* wnd, int w, int h) {
            DesktopWindowImpl* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
            Event event;
            event.type = Event::Resized;
            event.size.widht = w;
            event.size.heigth = h;

            window->m_event_queue.push(event);
        }

        void DesktopWindowImpl::maximized_callback(GLFWwindow* wnd, int state) {
            //c++ hack before c++ 17 to set unused parameter
            (void)(state);

            DesktopWindowImpl* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
            //c++ hack before c++ 17 to set unused parameter
            (void)(window);
        }

        void DesktopWindowImpl::setIcon(std::vector<robot2D::Texture>& icons) {
            if(icons.empty())
                return;
            std::vector<GLFWimage> images;
            for(auto &it: icons){
                GLFWimage img;
                auto size = it.get_size();
                img.width = size.x;
                img.height = size.y;
                img.pixels = it.get_pixels();
                images.emplace_back(img);
            }
            glfwSetWindowIcon(m_window, images.size(), &images[0]);
        }

        float DesktopWindowImpl::getDeltaTime() const {
            return glfwGetTime();
        }

        bool DesktopWindowImpl::isMousePressed(const Mouse& button) {
            return glfwGetMouseButton(m_window, button);
        }

        bool DesktopWindowImpl::isKeyboardPressed(const Key &key) {
            return glfwGetKey(m_window, key2Int(key));
        }

        void DesktopWindowImpl::dragdrop_callback(GLFWwindow* wnd, int count, const char** paths) {
            DesktopWindowImpl* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
            std::vector<std::string> outputPaths;
            for(int it = 0; it < count; ++it) {
                if(paths[it] != nullptr)
                    outputPaths.emplace_back(paths[it]);
            }
            if(window -> m_dragdrop_function)
                window -> m_dragdrop_function(outputPaths);
        }

        void DesktopWindowImpl::setDrapDropCallback(DrapDropCallback&& callback) {
            m_dragdrop_function = callback;
        }

        void DesktopWindowImpl::setMouseCursorVisible(const bool& flag) {
            m_cursorVisible = flag;
            glfwSetInputMode(m_window, GLFW_CURSOR, (m_cursorVisible ? GLFW_CURSOR_NORMAL: GLFW_CURSOR_HIDDEN));
        }

        void DesktopWindowImpl::focus_callback(GLFWwindow* wnd, int focus) {
            DesktopWindowImpl* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
            Event event{};
            if(focus) {
                event.type == Event::GainFocus;
            } else {
                event.type == Event::LostFocus;
            }

            window->m_event_queue.push(event);
        }

        void DesktopWindowImpl::text_callback(GLFWwindow *wnd, unsigned int c) {
            DesktopWindowImpl* window = static_cast<DesktopWindowImpl*>(glfwGetWindowUserPointer(wnd));
            Event event{};
            event.type = Event::TextEntered;
            event.text.symbol = c;
            window->m_event_queue.push(event);
        }

    }
}