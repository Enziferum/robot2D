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
            RB_CRITICAL("Robot2D::Exception: {0}", exception.what());
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