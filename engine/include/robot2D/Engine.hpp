#pragma once

#include <robot2D/Core/Clock.hpp>
#include <robot2D/Core/WindowContext.hpp>
#include <robot2D/Graphics/RenderWindow.hpp>
#include "Application.hpp"

namespace robot2D {

    struct EngineConfiguration {
        EngineConfiguration();
        EngineConfiguration(const vec2u& size,
                            const std::string& title,
                            const int& maxFrameRate = 60,
                            const WindowContext& windowContext = WindowContext::Default);

        static EngineConfiguration Default;

        vec2u windowSize;
        std::string windowTitle;
        int maxFrameRate;
        WindowContext windowContext;
    };

    class Engine {
    public:
        Engine();
        ~Engine() = default;

        void run(Application::Ptr application,
                 EngineConfiguration&& engineConfiguration);

    private:
        void setup();
    private:
        Application::Ptr m_application;
        Clock m_frameClock;
    };

    void robot2DInit();

    int runEngine(Application::Ptr application,
                  EngineConfiguration&& engineConfiguration = std::move(EngineConfiguration::Default));

#define ROBOT2D_RUN_ENGINE(AppClass, configuration) \
    AppClass::Ptr myApplication = std::make_unique<AppClass>(); \
    return robot2D::runEngine(std::move(myApplication), std::move(configuration));

#define ROBOT2D_MAIN(AppClass) \
int main() {         \
    AppClass::Ptr myApplication = std::make_unique<AppClass>(); \
    return robot2D::runEngine(std::move(myApplication)); \
}

}