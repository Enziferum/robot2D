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

#include <robot2D/Graphics/GL.h>

#include "robot2D/Core/Window.h"
#include "robot2D/Util/Logger.h"

namespace robot2D{

    constexpr int opengl_major = 3;
    constexpr int opengl_minor = 3;

    Window::Window():
    m_window(nullptr),
    m_win_size(800, 600),
    m_name("robot2D"),
    m_vsync(true) {
        setup();
    }

    Window::Window(const vec2u& size, const std::string& name, const bool& vsync):
            m_window(nullptr),
            m_win_size(size.x, size.y),
            m_name(name),
            m_vsync(vsync){
        setup();
    }


    Window::~Window() {
        glfwTerminate();
    }


    void Window::setup() {
        /* Initialize the library */
        if (!glfwInit())
            return;


        //todo window params
        /* Create a windowed mode window and its OpenGL context */
        m_window = glfwCreateWindow(m_win_size.x, m_win_size.y,
                                    m_name.c_str(), nullptr, nullptr);
        if (!m_window)
        {
            //todo throw expeption, after termimate
            glfwTerminate();
            exit(1);
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(m_window);
        if(m_vsync)
            glfwSwapInterval(1);

        setup_WGL();
        setup_callbacks();
    }

    void Window::setup_callbacks() {
        if(m_window == nullptr)
            return;

        glfwSetWindowUserPointer(m_window, this);
        glfwSetKeyCallback(m_window, key_callback);
        glfwSetCursorPosCallback(m_window, cursor_callback);
        glfwSetScrollCallback(m_window, mouseWhell_callback);
        glfwSetMouseButtonCallback(m_window, mouse_callback);
        glfwSetFramebufferSizeCallback(m_window, view_callback);
        glfwSetWindowSizeCallback(m_window, size_callback);
        glfwSetWindowMaximizeCallback(m_window, maximized_callback);
    }

    bool Window::isOpen() const {
        return !glfwWindowShouldClose(m_window);
    }

    bool Window::pollEvents(Event& event) {
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

    void Window::clear(const Color& color) {
        glClearColor(color.r, color.g, color.b, color.alpha);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Window::display() {
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }

    void Window::close() {
        glfwSetWindowShouldClose(m_window, 1);
    }

    void Window::key_callback(GLFWwindow* wnd, int key, int scancode,
                              int action, int mods) {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(wnd));

        Event event;
        if(action == GLFW_REPEAT || action == GLFW_PRESS)
            event.type = Event::KeyPressed;
        if(action == GLFW_RELEASE)
            event.type = Event::KeyReleased;
        event.key.code = key;
        window->m_event_queue.push(event);
    }

    void Window::cursor_callback(GLFWwindow* wnd, double xpos, double ypos) {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(wnd));

        Event event;
        event.move.x = float(xpos);
        event.move.y = float(ypos);
        event.type = Event::MouseMoved;

        window->m_event_queue.push(event);
    }

    void Window::mouseWhell_callback(GLFWwindow* wnd, double xpos, double ypos) {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(wnd));
        Event event;
        event.type = Event::MouseWheel;
        event.wheel.scroll_x = 0;
        event.wheel.scroll_y += float(ypos);
        window->m_event_queue.push(event);
    }

    void Window::setup_WGL() {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
             LOG_ERROR_E("Failed to initialize GLAD")
            return ;
        }

        //todo in rendertarget
        glViewport(0, 0, m_win_size.x, m_win_size.y);
    }

    void Window::mouse_callback(GLFWwindow* wnd, int button, int action, int mods) {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(wnd));

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

    const vec2u& Window::get_size() {
        return m_win_size;
    }

    void Window::view_callback(GLFWwindow* wnd, int w, int h) {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(wnd));
        Event event;
        event.type = Event::Resized;
        event.size.widht = w;
        event.size.heigth = h;
        window->m_win_size = vec2u((unsigned int)(w),
                                   (unsigned int)(h));
        window->onResize(w, h);
        window->m_event_queue.push(event);
    }

    void Window::onResize(const int &w, const int &h) {}


    bool Window::getMouseButton(const int &button) {
        return glfwGetMouseButton(m_window, button);
    }

    void Window::setCursorPosition(const vec2f &pos) {
        glfwSetCursorPos(m_window, pos.x, pos.y);
    }

    vec2f Window::getCursorPos() {
        vec2f pos;
        double x, y;
        glfwGetCursorPos(m_window, &x, &y);
        pos.x = x; pos.y = y;
        return pos;
    }

    void Window::size_callback(GLFWwindow* wnd, int w, int h) {
//        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(wnd));
//        Event event;
//        event.type = Event::Resized;
//        event.size.widht = w;
//        event.size.heigth = h;
//        window->m_event_queue.push(event);
    }

    void Window::maximized_callback(GLFWwindow* wnd, int state) {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(wnd));
        Event event;
    }

    GLFWwindow* Window::raw_window() const {
        return m_window;
    }

    void Window::setIcon(std::vector<Texture>& icons) {
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

}