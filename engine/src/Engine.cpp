#include <robot2D/Engine.hpp>
#include <robot2D/Util/Logger.hpp>

namespace robot2D {
    void robot2DInit() {
        logger::Log::Init();
    }

    EngineConfiguration EngineConfiguration::Default = EngineConfiguration();

    EngineConfiguration::EngineConfiguration():
    windowSize{800, 600},
    windowTitle{"Robot2D Engine"},
    maxFrameRate{60},
    windowContext{WindowContext::Default}
    {}

    EngineConfiguration::EngineConfiguration(
            const vec2u& size,
            const std::string& title,
            const int& FrameRate,
            const WindowContext& Context):

            windowSize{size},
            windowTitle{title},
            maxFrameRate{FrameRate},
            windowContext{Context}
            {}

    Engine::Engine():
    m_application{nullptr} {}

    void Engine::setup() {
        m_application -> setup();
        m_frameClock.restart();
    }

    void Engine::run(Application::Ptr application,
                     EngineConfiguration&& engineConfiguration) {

        m_application = std::move(application);

        RenderWindow renderWindow{engineConfiguration.windowSize,
                                  engineConfiguration.windowTitle,
                                  engineConfiguration.windowContext};

        float timePerFrame = 1.F / static_cast<float>(engineConfiguration.maxFrameRate);
        float timeProcessed = 0.F;

        m_application -> setWindow(&renderWindow);

        setup();
        while (m_application -> isRunning() &&
                renderWindow.isOpen()) {
            float delta = m_frameClock.restart().asSeconds();
            timeProcessed += delta;

            while (timeProcessed > timePerFrame) {
                timeProcessed -= timePerFrame;
                Event event{};
                while(renderWindow.pollEvents(event)) {
                    m_application -> handleEvents(event);
                }
                m_application -> handleMessages();
                m_application -> update(timePerFrame);
            }

            m_application -> guiUpdate(delta);
            m_application -> render();
        }

        m_application -> destroy();
    }

    int constructEngine(Application::Ptr application,
                         EngineConfiguration&& engineConfiguration) {
        Engine engine;

        try {
            engine.run(std::move(application),
                       std::move(engineConfiguration));
        }
        catch(const std::exception& exception) {
            RB_EDITOR_CRITICAL("Exception raised, reason: {0}", exception.what());
            return 1;
        }

        return 0;
    }

    int runEngine(Application::Ptr application,
                  EngineConfiguration&& engineConfiguration) {
        if(application == nullptr)
            return -1;

        return constructEngine(std::move(application),
                               std::move(engineConfiguration));
    }
}