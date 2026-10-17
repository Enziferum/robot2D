#include <robot2D/Engine.hpp>

class HelloWorld final : public robot2D::Application {
public:
    void setup() override {
        m_running = true;
    }

    void handleEvents(const robot2D::Event& event) override {
        if(event.getIf<robot2D::Event::Closed>())
            m_running = false;
    }

    void render() override {
        m_window -> clear(robot2D::Color::Black);
        m_window -> display();
    }
};

ROBOT2D_MAIN(HelloWorld)
