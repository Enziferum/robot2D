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

#include <cstdio>
#include <robot2D/Engine.hpp>
#include <robot2D/Util/Logger.hpp>

namespace robot2D {
    void robot2DInit() {
        logger::Log::Init();
    }

    EngineConfiguration EngineConfiguration::Default = EngineConfiguration();

    EngineConfiguration::EngineConfiguration() :
        windowSize{800, 600},
        windowTitle{"Robot2D Engine"},
        maxFrameRate{60},
        windowContext{WindowContext::Default} {
    }

    EngineConfiguration::EngineConfiguration(
        const vec2u& size,
        const std::string& title,
        const int& FrameRate,
        const WindowContext& Context) :

        windowSize{size},
        windowTitle{title},
        maxFrameRate{FrameRate},
        windowContext{Context} {
    }

    Engine::Engine() :
        m_application{nullptr} {
    }

    void Engine::setup() {
        m_application -> setup();
        m_frameClock.restart();
    }

    void Engine::run(Application::Ptr application,
        EngineConfiguration&& engineConfiguration) {

        m_application = std::move(application);

        RB_CORE_INFO("[Engine] Creating window; title='{}'; size={}x{}; frame_rate={}",
            engineConfiguration.windowTitle, engineConfiguration.windowSize.x,
            engineConfiguration.windowSize.y, engineConfiguration.maxFrameRate);

        DesktopRuntime desktop;
        RenderWindow renderWindow{desktop, engineConfiguration.windowSize,
                                  engineConfiguration.windowTitle,
                                  engineConfiguration.windowContext};

        struct ApplicationLifetime {
            Application::Ptr& application;
            RenderWindow& window;
            bool destroyStarted = false;
            ~ApplicationLifetime() noexcept {
                window.makeCurrent();
                if(application && !destroyStarted) {
                    try { application -> destroy(); }
                    catch(const std::exception& error) {
                        std::fprintf(stderr, "Application shutdown failed during unwinding: %s\n", error.what());
                    }
                    catch(...) { std::fputs("Application shutdown failed during unwinding: unknown exception\n",
                                     stderr); }
                }
                window.makeCurrent();
                application.reset();
            }
        } applicationLifetime{m_application, renderWindow};

        float timePerFrame = 1.F / static_cast<float>(engineConfiguration.maxFrameRate);
        float timeProcessed = 0.F;

        m_application -> setWindow(&renderWindow);

        RB_CORE_INFO("[Engine] Application setup started");
        setup();
        RB_CORE_INFO("[Engine] Application setup completed; entering main loop");
        std::uint64_t eventFrame = 0;
        while(m_application -> isRunning()) {
            desktop.processEvents();
            m_application -> handleEventFrame(renderWindow.takeEvents(++eventFrame));
            if(!m_application -> isRunning() || !renderWindow.isOpen()) break;
            float delta = m_frameClock.restart().asSeconds();
            timeProcessed += delta;

            while(timeProcessed >= timePerFrame) {
                timeProcessed -= timePerFrame;
                m_application -> handleMessages();
                m_application -> update(timePerFrame);
            }

            m_application -> guiUpdate(delta);
            m_application -> render();
        }

        RB_CORE_INFO("[Engine] Main loop stopped; application_running={}; window_open={}",
            m_application -> isRunning(), renderWindow.isOpen());
        RB_CORE_INFO("[Engine] Application destroy started");
        renderWindow.makeCurrent();
        applicationLifetime.destroyStarted = true;
        m_application -> destroy();
        RB_CORE_INFO("[Engine] Application destroy completed");
        // GUI and application graphics resources must be released before the local window.
        renderWindow.makeCurrent();
        m_application.reset();
        RB_CORE_INFO("[Engine] Application released; window can now be destroyed");
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
