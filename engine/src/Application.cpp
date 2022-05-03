#include <robot2D/Application.hpp>

namespace robot2D {
    Application::Application():
    m_running{true},
    m_window{nullptr}
    {}

    Application::~Application() {}

    void Application::destroy() {}

    void Application::handleEvents(const Event& event) {
        (void)event;
    }

    void Application::handleMessages() {}

    void Application::update(float deltaTime) {
        (void)deltaTime;
    }

    void Application::guiUpdate(float deltaTime) {
        (void)deltaTime;
    }

    void Application::setWindow(RenderWindow* window) {
        if(window == nullptr)
            return;
        m_window = window;
    }
}