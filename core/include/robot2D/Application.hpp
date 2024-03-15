#pragma once

#include <memory>
#include <robot2D/Core/Event.hpp>
#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Core/MessageBus.hpp>

namespace robot2D {

    class Application {
    public:
        using Ptr = std::unique_ptr<Application>;
    public:
        Application();
        virtual ~Application() = 0;

        virtual void setup() = 0;
        virtual void destroy();

        virtual void handleEvents(const robot2D::Event& event);
        virtual void handleMessages();
        virtual void update(float deltaTime);
        virtual void guiUpdate(float deltaTime);
        virtual void render() = 0;

        bool isRunning() const;
    protected:
        bool m_running;
        RenderWindow* m_window;
    private:
        friend class Engine;
        void setWindow(RenderWindow* window);
    };

    inline bool Application::isRunning() const {
        return m_running;
    }
}